# Private Computation Framework v1.0 (deprecated, please use v2.0)

## How PCF works
Private Computation Framework enables cryptographic methods that help two parties, Alice and Bob, compute a function on each of their secret inputs and receive outputs without revealing information about the inputs to each other. Specifically, it lets the programmers implement a garbled circuit-based 2pc program.

The garbled circuit protocol happens between a Garbler and an Evaluator, and the function they compute has to be described as a Boolean circuit. Garbler compiles the function into a Boolean circuit and adds their inputs to the circuit. Garbler then garbles the circuit, which hides the value of the inputs using encryption. Garbler sends the garbled circuit to Evaluator, who adds their inputs and computes the circuit. After the computation, Garbler and Evaluator will work together to get back the output of computation in clear text.

We looked into multiple services that enable secure computation to decide on the best framework that meets the requirements of Private RCT. After comparing more than twenty secure computation services — across secret sharing, garbled circuits, and homomorphic encryption-based protocols, we chose EMP-toolkit as the best fit for our application.

## Design Principles
1. Simplicity. We hide details of C++ and MPC as much as possible. We hope every engineer is able to write an MPC game with minimal ramp-up effort.
2. Efficiency. We provide many common utilities for engineers to write a game with ease. We embrace functional programming to speed up development.
3. Test and quality. We believe testing is important given the complexity of MPC. We provide a test framework for engineers to write unit tests to validate their games. We developed a special QueueIO for two parties to communicate via an in-memory queue. This allows MPC games to be implemented as pure functions, which also makes testing easier than before.

## Architecture

<img src="../pcfArch.jpg" alt="Figure 1: Architecture of an appplication on top of PCF" width="30%" height="30%">


### main
main is the main function in C++. It is responsible for dealing with command line arguments.

Note: AWS Sdk requires applications that use it to initialize it first ([Learn more](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/basic-use.html)). PCF provides a singleton that makes sure this is only performed once. It can be invoked by calling `fbpcf::AwsSdk::aquire()` in the main() function.

### EMPApp
We defined a unified interface. As you can read the code below, it’s responsible for dealing with input data and output data. It’s also responsible for launching a game.


```
template <class GameType, class InputDataType, class OutputDataType>

class EmpApp {
 public:
   virtual void run() {
 };

 protected:
   virtual InputDataType getInputData() = 0;
   virtual void putOutputData(const OutputDataType& output) = 0;

 private:
   Role role_;
   std::string serverIp_;
   uint16_t port_;
};
```

### EMPGame

EMPGame is another unified interface. It defines MPC role and exposes a `play` method which is as simple as taking some input data and return the output data after MPC computation.

```
enum class Role { Alice = emp::ALICE, Bob = emp::BOB };

template <class IOChannel, class InputDataType, class OutputDataType>
class EmpGame : public IMpcGame<InputDataType, OutputDataType> {
 protected:
   Role role_;

 private:
   std::unique_ptr<IOChannel> ioChannel_;
 };

class IMpcGame {
 public:
   virtual const OutputDataType play(const InputDataType& inputData) const = 0;
};
```
### Other components in PCF
**/mpc**
- EmpVector
- EmpTestUtil
- QueueIO

**/common**
- Functional map/reduce
- Vector operators

**/io**
- LocalFileManager
- S3FileManager
- FileManagerUtils
- MockedFileManger for testing

**/aws**
- AwsSdk singleton
- S3Util
- Mocked S3 client
