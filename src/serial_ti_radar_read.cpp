#include "ros/ros.h"
#include "serial/serial.h"
#include "sys/types.h"
#include <math.h>

//define 5 different messages and the buffer
uint8_t frame_header[52];
uint8_t frame_header_sync[8];
uint8_t tlv_header[8];

#define frame_header_package_length         *(uint32_t *)(frame_header + 20)
#define frame_header_frame_number           *(uint32_t *)(frame_header + 24)
#define frame_header_no_tlv                 *(uint16_t *)(frame_header + 48)

#define tlv_header_type                     *(uint32_t *)(tlv_header)
#define tlv_header_length                   *(uint32_t *)(tlv_header + 4)

#define tlv_header_type_pointCloud          0x06000000
#define tlv_header_type_targetObjectList    0x07000000
#define tlv_header_type_targetIndex         0x08000000

using namespace std;

int main(int argc, char *argv[])
{
	//creat ros handler to node
	ros::init(argc, argv, "serial_radar");
	ros::NodeHandle serial_radar_nh("~");

	serial::Serial fd;

	string serialPort;

	if(serial_radar_nh.getParam("serialPort", serialPort))
		printf("Retrived Port Name: %s\n", serialPort.data());
	else
	{
		printf("Cannot retrived Port name. Exit\n");
		exit(-1);
	}

	fd.setPort(serialPort.data());
	fd.setBaudrate(921600);
	fd.setTimeout(5, 10, 2, 10, 2);
	fd.open();
	if (fd.isOpen())
	{
		fd.flushInput();
		printf("Connection established\n\n");
	}
	else
	{
		printf("serialInit: Failed to open port\n");
		return 0;
	}

	ros::Rate rate(60);

	int lostsync = 0;
	uint8_t testsync[8];

	testsync[1] = 0x02;
	testsync[2] = 0x01;
	testsync[3] = 0x04;
	testsync[4] = 0x03;
	testsync[5] = 0x06;
	testsync[6] = 0x05;
	testsync[7] = 0x08;
	testsync[8] = 0x07;
	

	int got_frame_header = 0;
	int frame_header_bytes = 52;
	uint32_t target_frame_number = 0x00000000;
	uint32_t dataLength;
	int no_target;
	int no_input_points;

	while(ros::ok())
	{
		while(lostsync == 0)
		{
			if (got_frame_header == 0)
			{
				for (int i = 1; i <= 52; i++)
				{
					uint8_t temp_byte;
					fd.read(&temp_byte, 1);
					frame_header[i] = temp_byte;
				}
			}

			//check sync pattern
			for (int i = 0; i < 8; ++i)
			{
				if(frame_header[i] != frame_header_sync[i])
				{
					lostsync = 1;
					break;
				}
			}

			
			//check sum..................................................................................


			if (got_frame_header == 1)
			{
				//check for new sync
				if (frame_header_frame_number > target_frame_number)
				{
					target_frame_number = frame_header_frame_number;
					got_frame_header = 0;
				}
				else
				{
					lostsync = 1; //old frame
					got_frame_header = 0;
				}
			}

			if (lostsync)
				break;

			//we got a valid header
			dataLength = frame_header_package_length - 52;
			target_frame_number = frame_header_frame_number;

			no_input_points = 0;
			no_target = 0;

			for (int i = 1; i <= frame_header_frame_number; i++)
			{
				for (int i = 1; i <= 8; i++)
				{
					uint8_t temp_byte;
					fd.read(&temp_byte, 1);
					tlv_header[i] = temp_byte;
				}
				if (tlv_header_type == tlv_header_type_pointCloud)
				{
					/* code *///...................................................................................
				}
				else if (tlv_header_type == tlv_header_type_targetObjectList)
				{
					/* code */
				}
				else if (tlv_header_type == tlv_header_type_targetIndex)
				{
					/* code */
				}
			}



		}

		while(lostsync)
		{
			//Looking for sync data for the header
			int n = 1;
			while(n <= 8)
			{
				uint8_t temp_byte;
				fd.read(&temp_byte, 1);
				if(temp_byte == testsync[n])
					n++;
				else n = 1;
			}

			//Header is found, sync back
			if (n == 9)
			{
				lostsync = 0;
				//read header, 52-8 bytes
				for (int i = 9; i <= 52; i++)
				{
					uint8_t temp_byte;
					fd.read(&temp_byte, 1);
					frame_header[i] = temp_byte;
				}
				got_frame_header = 1;
			}
		}
		rate.sleep();
	}
	
}

