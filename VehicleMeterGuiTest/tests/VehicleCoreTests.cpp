#include <gtest/gtest.h>

#include "VehicleState.h"
#include "VehicleController.h"
#include "Config.h"


TEST(VehicleStateTest, InitialState)
{
    VehicleState state;

    EXPECT_EQ(state.power(), Power::Off);
    EXPECT_DOUBLE_EQ(state.speedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(state.fuelPercent(), 100.0);
    EXPECT_FALSE(state.accActive());
    EXPECT_EQ(state.driveMode(), DriveMode::Normal);
}


TEST(VehicleControllerTest, PowerOffIgnoresAccelerator)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    frame.accelerate = true;

    controller.update(frame, 0.1);

    EXPECT_DOUBLE_EQ(state.speedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(state.fuelPercent(), 100.0);
}


TEST(VehicleControllerTest, SpeedDoesNotExceedMaximum)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);

    frame.accelerate = true;

    for (int i = 0; i < 1000; ++i)
    {
        controller.update(frame, 0.1);
    }

    EXPECT_DOUBLE_EQ(
        state.speedKmh(),
        config::maxSpeedKmh
    );
}


TEST(VehicleStateTest, AccCannotEngageBelowMinimumSpeed)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(
        config::accMinimumKmh - 0.1
    );

    EXPECT_FALSE(state.engageAcc());
}


TEST(VehicleStateTest, AccCanEngageAtMinimumSpeed)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(
        config::accMinimumKmh
    );

    EXPECT_TRUE(state.engageAcc());

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        config::accMinimumKmh
    );
}

TEST(VehicleControllerTest, AccHoldsTargetSpeed)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(config::accMinimumKmh);

    ASSERT_TRUE(state.engageAcc());

    for (int i = 0; i < 100; ++i)
    {
        controller.update(frame, 0.1);
    }

    EXPECT_NEAR(
        state.speedKmh(),
        config::accMinimumKmh,
        0.001
    );
}


TEST(VehicleControllerTest, AccReturnsToTargetSpeed)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(config::accMinimumKmh);

    ASSERT_TRUE(state.engageAcc());

    state.setSpeedKmh(50.0);

    for (int i = 0; i < 100; ++i)
    {
        controller.update(frame, 0.1);
    }

    EXPECT_NEAR(
        state.speedKmh(),
        config::accMinimumKmh,
        0.01
    );
}


TEST(VehicleControllerTest, BrakeCancelsAcc)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(60.0);

    ASSERT_TRUE(state.engageAcc());

    frame.accelerate = true;
    frame.brake = true;

    controller.update(frame, 0.1);

    EXPECT_FALSE(state.accActive());
    EXPECT_LT(state.speedKmh(), 60.0);
}


TEST(VehicleControllerTest, PowerOffClearsSpeedAndAcc)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(60.0);

    ASSERT_TRUE(state.engageAcc());

    frame.commands = {
        Command::TogglePower
    };

    controller.update(frame, 0.1);

    EXPECT_EQ(state.power(), Power::Off);
    EXPECT_DOUBLE_EQ(state.speedKmh(), 0.0);
    EXPECT_FALSE(state.accActive());
    EXPECT_DOUBLE_EQ(state.accTargetKmh(), 0.0);
}


TEST(VehicleControllerTest, EmptyFuelCancelsAcc)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setFuelPercent(0.000001);
    state.setSpeedKmh(60.0);

    ASSERT_TRUE(state.engageAcc());

    controller.update(frame, 0.1);

    EXPECT_DOUBLE_EQ(state.fuelPercent(), 0.0);
    EXPECT_FALSE(state.accActive());
}

TEST(VehicleControllerTest, ResetRestoresInitialStateAndEconomy)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(60.0);
    state.setFuelPercent(50.0);

    frame.accelerate = true;

    for (int i = 0; i < 10; ++i)
    {
        controller.update(frame, 0.1);
    }

    frame = {};
    frame.commands = {
        Command::Reset
    };

    controller.update(frame, 0.1);

    EXPECT_EQ(state.power(), Power::Off);
    EXPECT_DOUBLE_EQ(state.speedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(state.fuelPercent(), 100.0);
    EXPECT_FALSE(state.accActive());

    EXPECT_DOUBLE_EQ(
        controller.economy().distanceKm(),
        0.0
    );

    EXPECT_FALSE(
        controller.economy().averageKmPerLiter().has_value()
    );
}


TEST(VehicleStateTest, FuelPercentDoesNotExceedMaximum)
{
    VehicleState state;

    state.setFuelPercent(150.0);

    EXPECT_DOUBLE_EQ(
        state.fuelPercent(),
        100.0
    );
}


TEST(VehicleStateTest, FuelPercentDoesNotGoBelowZero)
{
    VehicleState state;

    state.setFuelPercent(-10.0);

    EXPECT_DOUBLE_EQ(
        state.fuelPercent(),
        0.0
    );
}

TEST(FuelEconomyCalculatorTest, CalculatesDistanceCorrectly)
{
    VehicleState state;
    state.setPower(Power::On);

    FuelEconomyCalculator economy;

    economy.update(
        state,
        60.0,
        false,
        60.0
    );

    EXPECT_NEAR(
        economy.distanceKm(),
        1.0,
        1e-9
    );
}


TEST(FuelEconomyCalculatorTest, CalculatesConsumedFuelCorrectly)
{
    VehicleState state;
    state.setPower(Power::On);

    FuelEconomyCalculator economy;

    economy.update(
        state,
        60.0,
        false,
        60.0
    );

    const double expectedLiters =
        (0.6 + 60.0 / 18.0) / 60.0;

    EXPECT_NEAR(
        economy.consumedLiters(),
        expectedLiters,
        1e-9
    );
}


TEST(FuelEconomyCalculatorTest, CalculatesAverageFuelEconomy)
{
    VehicleState state;
    state.setPower(Power::On);

    FuelEconomyCalculator economy;

    economy.update(
        state,
        60.0,
        false,
        60.0
    );

    const double expectedLiters =
        (0.6 + 60.0 / 18.0) / 60.0;

    ASSERT_TRUE(
        economy.averageKmPerLiter().has_value()
    );

    EXPECT_NEAR(
        economy.averageKmPerLiter().value(),
        1.0 / expectedLiters,
        1e-9
    );
}

TEST(VehicleStateTest, AccTargetRoundsUpToNextStep)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(68.0);

    ASSERT_TRUE(state.engageAcc());

    state.increaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        70.0
    );
}


TEST(VehicleStateTest, AccTargetRoundsDownToPreviousStep)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(68.0);

    ASSERT_TRUE(state.engageAcc());

    state.decreaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        65.0
    );
}


TEST(VehicleStateTest, AccTargetIncreasesByStepWhenAlreadyAligned)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(70.0);

    ASSERT_TRUE(state.engageAcc());

    state.increaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        70.0 + config::accSetStep
    );
}


TEST(VehicleStateTest, AccTargetDecreasesByStepWhenAlreadyAligned)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(70.0);

    ASSERT_TRUE(state.engageAcc());

    state.decreaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        70.0 - config::accSetStep
    );
}

TEST(VehicleStateTest, AccTargetDoesNotExceedMaximumSpeed)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(config::maxSpeedKmh);

    ASSERT_TRUE(state.engageAcc());

    state.increaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        config::maxSpeedKmh
    );
}


TEST(VehicleStateTest, AccTargetDoesNotGoBelowMinimumSpeed)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(config::accMinimumKmh);

    ASSERT_TRUE(state.engageAcc());

    state.decreaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        config::accMinimumKmh
    );
}

TEST(VehicleStateTest, AccTargetChangeIsIgnoredWhenAccIsInactive)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setSpeedKmh(60.0);

    state.increaseAccTarget();
    state.decreaseAccTarget();

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        0.0
    );
}

TEST(VehicleControllerTest, IncreaseAccTargetCommandUsesRounding)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(68.0);

    ASSERT_TRUE(state.engageAcc());

    frame.commands = {
        Command::IncreaseAccTarget
    };

    controller.update(frame, 0.1);

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        70.0
    );
}


TEST(VehicleControllerTest, DecreaseAccTargetCommandUsesRounding)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(68.0);

    ASSERT_TRUE(state.engageAcc());

    frame.commands = {
        Command::DecreaseAccTarget
    };

    controller.update(frame, 0.1);

    EXPECT_DOUBLE_EQ(
        state.accTargetKmh(),
        65.0
    );
}

TEST(DriveModeTest, InitialModeIsNormal)
{
    VehicleState state;

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Normal
    );
}


TEST(DriveModeTest, ModeDoesNotChangeWhilePowerIsOff)
{
    VehicleState state;

    state.cycleDriveMode();

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Normal
    );
}


TEST(DriveModeTest, ModeCyclesNormalSportEcoNormal)
{
    VehicleState state;

    state.setPower(Power::On);

    state.cycleDriveMode();

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Sport
    );

    state.cycleDriveMode();

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Eco
    );

    state.cycleDriveMode();

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Normal
    );
}


TEST(DriveModeTest, PowerOffResetsModeToNormal)
{
    VehicleState state;

    state.setPower(Power::On);

    state.cycleDriveMode();

    ASSERT_EQ(
        state.driveMode(),
        DriveMode::Sport
    );

    state.setPower(Power::Off);

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Normal
    );
}


TEST(VehicleControllerTest, ChangeDriveModeCommandChangesMode)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);

    frame.commands = {
        Command::ChangeDriveMode
    };

    controller.update(frame, 0.1);

    EXPECT_EQ(
        state.driveMode(),
        DriveMode::Sport
    );
}


TEST(DriveModeTest, DriveModeChangesAcceleration)
{
    VehicleState normalState;
    VehicleState sportState;
    VehicleState ecoState;

    normalState.setPower(Power::On);
    sportState.setPower(Power::On);
    ecoState.setPower(Power::On);

    sportState.cycleDriveMode();

    ecoState.cycleDriveMode();
    ecoState.cycleDriveMode();

    ASSERT_EQ(
        normalState.driveMode(),
        DriveMode::Normal
    );

    ASSERT_EQ(
        sportState.driveMode(),
        DriveMode::Sport
    );

    ASSERT_EQ(
        ecoState.driveMode(),
        DriveMode::Eco
    );

    VehicleController normalController(normalState);
    VehicleController sportController(sportState);
    VehicleController ecoController(ecoState);

    InputFrame frame;
    frame.accelerate = true;

    normalController.update(frame, 1.0);
    sportController.update(frame, 1.0);
    ecoController.update(frame, 1.0);

    EXPECT_LT(
        ecoState.speedKmh(),
        normalState.speedKmh()
    );

    EXPECT_LT(
        normalState.speedKmh(),
        sportState.speedKmh()
    );
}

TEST(VehicleStateTest, AccCannotEngageWhilePowerIsOff)
{
    VehicleState state;

    state.setSpeedKmh(60.0);

    EXPECT_FALSE(state.engageAcc());
}


TEST(VehicleStateTest, AccCannotEngageWithEmptyFuel)
{
    VehicleState state;

    state.setPower(Power::On);
    state.setFuelPercent(0.0);
    state.setSpeedKmh(60.0);

    EXPECT_FALSE(state.engageAcc());
}


TEST(VehicleControllerTest, EmptyFuelCausesVehicleToCoast)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setFuelPercent(0.0);
    state.setSpeedKmh(60.0);

    frame.accelerate = true;

    controller.update(frame, 0.1);

    EXPECT_LT(state.speedKmh(), 60.0);
    EXPECT_DOUBLE_EQ(state.fuelPercent(), 0.0);
}


TEST(FuelEconomyCalculatorTest, InstantEconomyIsUnavailableWhilePowerIsOff)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    controller.update(frame, 0.1);

    EXPECT_FALSE(
        controller.economy()
        .instantKmPerLiter()
        .has_value()
    );
}


TEST(VehicleControllerTest, BrakeReducesSpeedToZero)
{
    VehicleState state;
    VehicleController controller(state);
    InputFrame frame;

    state.setPower(Power::On);
    state.setSpeedKmh(60.0);

    frame.brake = true;

    for (int i = 0; i < 1000; ++i)
    {
        controller.update(frame, 0.1);
    }

    EXPECT_DOUBLE_EQ(
        state.speedKmh(),
        0.0
    );
}