# User Programming Handbook for Developing MPC Application on PCF v2.0

## Application Development Philosophy

In PCF 2.0, all parties execute *exactly the same* application code, including input process.  Code will describe the application from an objective point of view, *without dependency on which party is executing the code*.

For example, a secret bit from party ALICE with value v will be represented as “SecBit bit(v, ALICE)”.  All parties will execute this line of code.  Party Alice will use her actual secret value v as input while other parties can use any (valid) dummy value as their inputs (these values will be ignored in the backend).

Below is the example code for billionaire problem:

    bool billionaireProblem(int asset1Plaintext, int asset2Plaintext) {
        SecInt asset1(asset1Plaintext, party1);
        SecInt asset2(asset2Plaintext, party2);
        return (asset1 < asset2).openToParty(party1);
    }

Party 1 will run this code as `billionaireProblem(myAsset, dummyValue)` while party 2 will run it as `billionaireProblem(dummyValue, myAsset)`.

## How to develop apps in PCF v2.0

### SchedulerID

All applications developed within PCF 2.0 are **backend independent**. The concrete backend crypto protocol can be decided by configurations used at runtime. Besides, PCF 2.0 supports co-existing backends. Multiple applications can execute with their respective backends in parallel. Naturally, there is a way to specify which backend should each application execute on. This is done by adding a template parameter `schedulerId` to all private data types (see details below). **Private computation can only happen between private types with the same `schedulerId`**. Trying to write computation between private types with different `schedulerId` will result in compile time error, since it makes no sense to the backend.

### isSecret

All private types are marked as either secret or public by a template parameter. No party is expected to learn the secret held by the private type if it is marked secret (the only exception is this secret value is some party's input). Therefore there will be a compile time error if one tries to call `getValue()` on secret values. Instead, they should firstly use `openToParty()` to convert a secret value to a public value beforehand.

### usingBatch

In some scenarios, a large computation is merely massive duplications of a pretty small gadget over different inputs. `usingBatch` flag, as a template parameter, indicates whether the computation should be executed multiple times with different inputs.  The use of `usingBatch` will significantly speed things up.

### Types

As we discussed above, all types provided by PCF 2.0 are templates. There are at least three template parameters as explained above: `bool isSecret`, `int schedulerId`, and `bool usingBatch`. Below is a brief summary.

- `isSecret` controls whether the variable is a public value known to everyone or a secret value known to no one (perhaps except its owner);
- `schedulerId` controls which backend instance this variable will use to run computation;
- `usingBatch` controls whether this variable represents a single value or a batch of values.

Currently PCF 2.0 provides three type templates: `Bit`, `Int`, and `BitString`.

- `Bit` represents one bit (a batch of bits if usingBatch = true) . Currently we support the following operators: !, ^, ||, and & in addition to standard copy/move operations.
- `Int` represents a signed or an unsigned int of certain width (a batch of ints if usingBatch = true) . Currently we support the following operators: +, -, <, ==, and mux in addition to standard copy/move operations.

Application developers can define their own type templates (alias). But any user-defined template must have *isScheduler* as a template variable and use it in any underlying native templates (Bit and Int).

### Testing with a single caller

Previously, private applications must use a secure but inconvenient backend to test the correctness of their own logic. This requirement makes it hard to debug and test the application.

In PCF v2.0, a plaintext backend is provided for developing applications. With this backend, applications can execute in cleartext without suffering the overhead of backend cryptographic protocols. In addition, they can be tested in plaintext with only one caller. After setting the backend up, the application can be tested with the caller providing all test values for input.

For example, the example billionaire problem code can be tested as `billionaireProblem(testValue1, testValue2)`.

## Running with Multiple Parties

The steps of running applications in a distributed environment with multiple parties is very similar to that of testing with a single caller. The first step is to jointly set up the proper backend among all parties. Then each party can execute the application with actual values for their own inputs and dummy value as placeholders for other parties' inputs.
