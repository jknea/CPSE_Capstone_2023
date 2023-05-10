/* 
 * Author: James Knea
 * Last Modified Date: 5/3/2023
 *
 * Description:
 *   This code to test Teensy 4.1 MCU communication on three CAN buses.
 *   CAN1 and CAN3 are used for controlling RMDx8-Pro motors, and CAN2 is used to communicate with another Teensy MCU.
 */

#include <FlexCAN_T4.h>

#define BUFF_SIZE     8
#define PORT          8888
#define CAN_1M        1000000
#define DELAY_MS      1

/* CANBus Variables */
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;  // can1 port 
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;  // can2 port 
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;  // can3 port 

const int RMD_CAN_ID0 = 0x141;    // RMD ID for CAN1
const int RMD_CAN_ID1 = 0x145;    // RMD ID for CAN3
const int IMU_CAN_ID = 0x300;     // IMU ID for CAN2

CAN_message_t zero_torque0, zero_torque1, imu_command;  // buffers to send data
CAN_message_t msg_recv1, msg_recv2, msg_recv3;          // buffers to receive data


void setup() 
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("serial init!");
  initCAN();
  delay(2000);
}

int ok = 0;
bool flag1 = 0, flag2 = 0, flag3 = 0;

void loop() 
{ 
  send_all();
  recv_all();
  delay(DELAY_MS);
}



void send_all()
{
  while(! (flag1 && flag2 && flag3))
  {
    if (!flag1) 
    {
      ok = can1.write(zero_torque0);
      if (ok == 1)
      {
        flag1 = 1;
      }
    }
    if (!flag2) 
    {
      ok = can2.write(imu_command);
      if (ok == 1)
      {
        flag2 = 1;
      }
    }
    if (!flag3) 
    {
      ok = can3.write(zero_torque1);
      if (ok == 1)
      {
        flag3 = 1;
      }
    }
  }
  // clear flags
  flag1 = 0;
  flag2 = 0;
  flag3 = 0;
}




void recv_all()
{
  while(! ( flag3 && flag2 && flag1))
  {
    if (!flag1) 
    {
      ok = can1.read(msg_recv1);
      if (ok == 1)
      {
        flag1 = 1;
      }
    }
    if (!flag2) 
    {
      ok = can2.read(msg_recv2);
      if (ok == 1)
      {
        flag2 = 1;
      }
    }
    if (!flag3) 
    {
      ok = can3.read(msg_recv3);
      if (ok == 1)
      {
        flag3 = 1;
      }
    }
  }
  flag1 = 0;
  flag2 = 0;
  flag3 = 0;
}



void wait(unsigned us) {
  unsigned long T0 = micros();
  while ((micros() - T0) < us);
}


void initCAN() {
  zero_torque0.id = RMD_CAN_ID0;
  zero_torque1.id = RMD_CAN_ID1;
  imu_command.id = IMU_CAN_ID;

  zero_torque0.buf[0] = 0xA1;
  zero_torque1.buf[0] = 0xA1;
  imu_command.buf[0] = 0;

  for (int i = 1; i < 8; ++i) {
    zero_torque0.buf[i] = 0;
    zero_torque1.buf[i] = 0;
    imu_command.buf[i] = i;
  }

  can1.begin();
  can2.begin();
  can3.begin();
  can1.setBaudRate(CAN_1M);     // 1 Mbps data rate
  can2.setBaudRate(CAN_1M);   
  can3.setBaudRate(CAN_1M);     // 1 Mbps data rate
}
