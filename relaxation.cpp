/**
 * Simulation of a relaxation oscillator
 *
 * Copyright 2020  Jason Leake
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>

using floating = double;

/**
 * This class represents the capacitor.  It is an ideal device without
 * any resistance, leakage etc.
 */
class Capacitor {
private:
  // Capaitor value
  const floating capacitance;
  // Charge stored in the capacitor
  floating totalCharge;
public:
  // Constructor.  Initial charge is zero.
  Capacitor(floating capacitance) : capacitance(capacitance) {}
  virtual ~Capacitor() = default;

  /**
   * Add charge to the capacitor
   *
   * @param charge to add. Can be positive or negative.
   */
  void addCharge(floating charge) {
    totalCharge += charge;    
  }

  /**
   * Get the voltage across the capacitor
   *
   * @return voltage across capacitor
   */
  auto voltage() const {
    return totalCharge / capacitance;
  }

};

/**
 * This class represents the resistance
 */
class Resistor {
private:
  const floating resistance;
public:
  /**
   * Constructor
   */
  Resistor(floating resistance) : resistance{resistance} {}
  virtual ~Resistor() = default;

  /**
   * Calculate the current flowing through the resistor.
   *
   * @param v1 voltage one side of the resistor
   * @param v2 voltage on the other side of the resistor
   * @return current through the resistor
   */
  auto current(floating v1, floating v2) const {
    return (v1 - v2) / resistance;
  }

};


/**
 * This class simulates the inverter.  It has a Schmitt trigger, so
 * that the voltage threshold for transitioning to a low input state
 * is different to that for transitioning to a high input state.  It
 * is an ideal device, with zero propagation delay, infinite input
 * impedance and zero output impedance.
 */
class Inverter {
private:
  // Output voltage in high state
  floating outputHi;
  // Output voltage in low state
  floating outputLow;
  // Input voltage needed for transition of input from high to low state
  floating hiLowTransition;
  // Input voltage needed for transistion of input from low to high state
  floating lowHiTransition;
  // Current output state - true if high
  bool hi;
  
public:
  /**
   * Constructor. Initial state is low.
   *
   * @param outputLow low state output voltage
   * @param outputHi high state output voltage
   * @param hiLowTransition transition voltage to take input from high to low
   * @param lowHiTransition transition voltage to take input from low to high
   */
  Inverter(floating outputLow,
	   floating outputHi,
	   floating hiLowTransition,
	   floating lowHiTransition,
	   floating voltage) :
    outputHi{outputHi},
    outputLow{outputLow},
    hiLowTransition{hiLowTransition},
    lowHiTransition{lowHiTransition} {
    setInputVoltage(voltage);
  }

  /**
   * Set the input voltage.
   *
   * @param voltage input voltage
   */
  void setInputVoltage(floating voltage) {
    if (!hi) {
      if (voltage <= hiLowTransition) {
	// Input state is false, so output state is true
	hi = true;
      }
    }
    else if (voltage >= lowHiTransition) {
      // Input state is high, so output state is false
      hi = false;
    }
  }

  /**
   * Get the output voltage
   *
   * @return output voltage
   */
  auto getOutputVoltage() const {
    // High state produces high output voltage, low state produces low output
    // voltage
    return hi ? outputHi : outputLow;
  }
};


/**
 * This class keeps track of the current state, recording the time interval
 * since the previous state change
 */
class StateMonitor {
private:
  std::optional<floating> highInterval;
  std::optional<floating> lowInterval;
  std::optional<floating> previousTime;

public:
  /**
   * Flag a state change.
   *
   * @param stateHigh true if state changed to high, false if changed to low
   * @param time at which the state changed
   */
  auto stateChange(bool stateHigh, floating time) {
    if (stateHigh) {
      std::cout << "Signal high at " << time;
    }
    else {
      std::cout << "Signal low at " << time;
    }
    if (previousTime.has_value()) {
      auto interval = time - previousTime.value();
      std::cout << ", interval since last state change = "
		<< interval << " seconds";
      if (stateHigh) {
	if (!lowInterval.has_value()) {
	  lowInterval = interval;
	}
      }
      else if (!highInterval.has_value()) {
	highInterval = interval;
      }
    }
    std::cout << "\n";
    previousTime = time;
  }

  /**
   * Return the cycle period
   *
   * @return cycle period in seconds
   */
  auto getPeriod() const {
    return highInterval.value() + lowInterval.value();
  }
};

/**
 * Convert text field to numeric value.  e.g. "1.23e-6" to a floating
 * point value.  Throws an exception and terminates program if this isn't
 * possible.
 *
 * @param field text field to convert
 * @return numeric value
 */
auto convert(const char* field) {
  try {
    auto retval = std::stod(field);
    return static_cast<floating>(retval);
  }
  catch (std::invalid_argument& e) {
    std::cerr << "Unable to convert \"" << field << "\" to numeric\n";
    std::exit(EXIT_FAILURE);
  }
}


int main(int argc, char** argv) {

  // Writes this file containing per-timestep simulation values 
  constexpr auto CSV_FILENAME = "output.csv";
  // Writes this file containing description of settings and derived frequency
  constexpr auto DESCR_FILENAME = "description.dat";

  // Default values
  auto resistance = floating{1e3};
  auto capacitance = floating{1e-7};
  auto logicLowVoltage = floating{0};
  auto logicHighVoltage = floating{5.0};
  auto logicLowTransitionVoltage = floating{0.6};
  auto logicHighTransitionVoltage = floating{2.5};

  if (argc > 1) {
    resistance = convert(argv[1]);
  } else {
    std::cout << "Using default resistance\n";
  }
  
  if (argc > 2) {
    capacitance = convert(argv[2]);
  } else {
    std::cout << "Using default capacitance\n";
  }
  
  if (argc > 3) {
    logicLowVoltage = convert(argv[3]);
  } else {
    std::cout << "Using default low voltage\n";
  }
  
  
  if (argc > 4) {
    logicHighVoltage = convert(argv[4]);
  } else {
    std::cout << "Using default high voltage\n";
  }
  
  if (argc > 5) {
    logicLowTransitionVoltage = convert(argv[5]);
  }
   else {
    std::cout << "Using default high->low state transition voltage\n";
  }
  
  if (argc > 6) {
    logicHighTransitionVoltage = convert(argv[6]);
  } else {
    std::cout << "Using default low->high state transition voltage\n";
  }
  
  std::cout << "\n";
  std::cout << "R = " << resistance << " ohms\n";
  std::cout << "C = " << capacitance << " farads\n";
  std::cout << "Logic high = " << logicHighVoltage << " volts\n";
  std::cout << "Logic low = " << logicLowVoltage << " volts\n";
  std::cout << "Logic high to low transition = " << logicHighTransitionVoltage
	    << " volts\n";
  std::cout << "Logic low to high transition = " << logicLowTransitionVoltage
	    << " volts\n";
  std::cout << "\n";

  auto descriptionFile = std::ofstream(DESCR_FILENAME);
  if (descriptionFile) {
    descriptionFile << "FILE = " << CSV_FILENAME << "\n";
    descriptionFile << "RESISTANCE = " << resistance << "\n";
    descriptionFile << "CAPACITANCE = " << capacitance << "\n";
    descriptionFile << "LH = " << logicHighVoltage << "\n";
    descriptionFile << "LL = " << logicLowVoltage << "\n";
    descriptionFile << "LLT = " << logicHighTransitionVoltage << "\n";
    descriptionFile << "LHT = " << logicLowTransitionVoltage << "\n";
  } else {
    std::cerr << "Unable to open " << DESCR_FILENAME << " for writing\n";
  }
  
  auto inverter = Inverter{logicLowVoltage,
			   logicHighVoltage,
			   logicLowTransitionVoltage,
			   logicHighTransitionVoltage, 0};
  auto capacitor = Capacitor{capacitance};
  auto resistor = Resistor{resistance};

  auto timeConstant = capacitance * resistance;
  std::cout << "Approx time constant is " << timeConstant
	    << " seconds\n";
  // Plot about ten cycles.  Each cycle is a million timesteps.
  auto timestepSize = 1e-4 * timeConstant;
  auto numberOfTimesteps = 10 * timeConstant / timestepSize;

  std::cout << "Run for " << numberOfTimesteps * timestepSize << " seconds\n";
  std::cout << "Timestep size is " << timestepSize << " seconds\n";
  
  auto out = std::ofstream{CSV_FILENAME};
  if (!out.is_open()) {
    std::cerr << "Unable to open " << CSV_FILENAME << " for writing\n";
    return EXIT_FAILURE;
  }
  std::cout << "\n";

  auto stateMonitor = StateMonitor();
  auto lastInverterOutput = std::optional<floating>{};
  for (auto step = 0; step < numberOfTimesteps; step++) {
    auto capacitorVoltage = capacitor.voltage();
    auto inverterVoltage = inverter.getOutputVoltage();
    auto current = resistor.current(inverterVoltage, capacitorVoltage);

    // Linear approximate the charge flowing into the capacitor
    auto charge = current * timestepSize;
    capacitor.addCharge(charge);
    inverter.setInputVoltage(capacitor.voltage());
    auto inverterOutput = inverter.getOutputVoltage();
    out << step * timestepSize << "," <<
      capacitor.voltage() <<
      ", " << inverterOutput << "\n";

    if (!lastInverterOutput.has_value()) {
      lastInverterOutput = inverterOutput;
    }
    else if (lastInverterOutput != inverterOutput) {
      lastInverterOutput = inverterOutput;
      stateMonitor.stateChange(inverterOutput == logicHighVoltage,
			       step * timestepSize);
    }
  }
  std::cout << "\n";

  auto frequency = 1./stateMonitor.getPeriod();
  std::cout << "Frequency is " << frequency << " Hz" << std::endl;
  descriptionFile << "FREQUENCY = " << frequency << "\n";
  descriptionFile.close();

  return EXIT_SUCCESS;
}
