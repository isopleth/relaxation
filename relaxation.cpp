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

class Capacitor {
private:
  floating capacitance;
  floating totalCharge;
public:
  Capacitor(floating capacitance) : capacitance(capacitance) {}
  virtual ~Capacitor() = default;
  
  void addCharge(floating charge) {
    totalCharge += charge;    
  }

  auto voltage() {
    return totalCharge / capacitance;
  }

  
  auto getCapacitance() const {
    return capacitance;
  }
};

class Resistor {
private:
  const floating resistance;
public:
  Resistor(floating resistance) : resistance{resistance} {}
  virtual ~Resistor() = default;

  auto current(floating v1, floating v2) const {
    return (v1 - v2) / resistance;
  }

  auto getResistance() const {
    return resistance;
  }
};


class Inverter {
private:
  floating outputHi;
  floating outputLow;
  floating hiLowTransition;
  floating lowHiTransition;
  bool hi;
  
public:
  Inverter(floating outputLow,
	   floating outputHi,
	   floating hiLowTransition,
	   floating lowHiTransition,
	   floating voltage) :
    outputHi{outputHi},
    outputLow{outputLow},
    hiLowTransition{hiLowTransition},
    lowHiTransition{lowHiTransition}
  {
    setInputVoltage(voltage);
  }

  auto setState(bool state) {
    hi = state;
  }
  
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

  auto getOutputVoltage() const {
    return hi ? outputHi : outputLow;
  }
};

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

auto stateChange(bool stateHigh, floating time) {
  static auto previousTime = std::optional<floating>{};
  if (stateHigh) {
    std::cout << "Signal high at " << time;
  }
  else {
    std::cout << "Signal low at " << time;
  }
  if (previousTime.has_value()) {
    auto interval = time - previousTime.value();
    std::cout << ", interval = " << interval << " seconds";
  }
  std::cout << "\n";
  previousTime = time;
}


int main(int argc, char** argv) {

  constexpr auto CSV_FILENAME = "output.csv";
  constexpr auto DESCR_FILENAME = "description.dat";

  // Default values
  auto resistance = floating{1e4};
  auto capacitance = floating{1e-6};
  auto logicLowVoltage = floating{0};
  auto logicHighVoltage = floating{5.0};
  auto logicLowTransitionVoltage = floating{0.6};
  auto logicHighTransitionVoltage = floating{2.5};

  if (argc > 1) {
    resistance = convert(argv[1]);
  }
  if (argc > 2) {
    capacitance = convert(argv[2]);
  }
  if (argc > 3) {
    logicLowVoltage = convert(argv[3]);
  }
  if (argc > 4) {
    logicHighVoltage = convert(argv[4]);
  }
  if (argc > 5) {
    logicLowTransitionVoltage = convert(argv[5]);
  }
  if (argc > 6) {
    logicHighTransitionVoltage = convert(argv[6]);
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
    descriptionFile << "F = " << CSV_FILENAME << "\n";
    descriptionFile << "R = " << resistance << "\n";
    descriptionFile << "C = " << capacitance << "\n";
    descriptionFile << "LH = " << logicHighVoltage << "\n";
    descriptionFile << "LL = " << logicLowVoltage << "\n";
    descriptionFile << "LLT = " << logicHighTransitionVoltage << "\n";
    descriptionFile << "LHT = " << logicLowTransitionVoltage << "\n";
  }
  else {
    std::cerr << "Unable to open " << DESCR_FILENAME << " for writing\n";
  }
  descriptionFile.close();
  
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
  
  auto lastInverterOutput = std::optional<floating>{};
  for (auto step = 0; step < numberOfTimesteps; step++) {
    auto capacitorVoltage = capacitor.voltage();
    auto inverterVoltage = inverter.getOutputVoltage();
    auto current = resistor.current(inverterVoltage, capacitorVoltage);
    // std::cout << "Current is " << current << "\n";
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
      stateChange(inverterOutput == logicHighVoltage, step * timestepSize);
    }
  }
  std::cout << "\n";
  
  return EXIT_SUCCESS;
}
