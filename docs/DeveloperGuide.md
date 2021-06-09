# Develop an MPC app step by step
## How to write an MPC game?
There are two steps
1. Define input data type and output data type
2. Inherit EmpGame and implement the `play` method

## How to unit test an MPC game?
We built QueueIO and EmpTestUtil to help you to build unit tests. Following is the classic millionaire example.

```
class MillionaireGame : public EmpGame<QueueIO, int, bool> {
 public:
  MillionaireGame(std::unique_ptr<QueueIO> io, Role role): EmpGame<QueueIO, int, bool>(std::move(io), role) {}

  const bool play(const int& number) const override {
    emp::Integer a{64, number, emp::ALICE};
    emp::Integer b{64, number, emp::BOB};
    return (a > b).reveal<bool>();
  }
};

TEST(MillionaireGame, AliceIsRicher) {
 //Alice has 2 and Bob has 1
 auto res = EmpTestUtil::test<MillionaireGame, int, bool>(2, 1);
 EXPECT_EQ(true, res.first);
 EXPECT_EQ(true, res.second);
}
```
## How to implement an MPC application?
+Simply inherit `EmpApp` and implement `getInputData()` and `putOutputData()`. The default `run()` should be able to handle most games, otherwise, feel free to override it.

## How to build integration tests for an MPC application?
As EmpApp is responsible for IO, we recommend writing integration tests instead of unit tests (example).

## How to run E2E test on AWS?
MPC service is able to schedule your MPC application on AWS. Before onboarding your app to MPC service, we recommend you run an E2E test to ensure your app is able to run on AWS without issues. We leverage our newly built One Docker test framework to perform E2E tests. There are two steps;
1. Upload your binary to One Docker repository.
```onedocker-cli upload --package_name --package_dir```
2. Launch one game for each party (two games in total). Wait for them to complete and check results.
```onedocker-cli test --package_name --cmd_args```
