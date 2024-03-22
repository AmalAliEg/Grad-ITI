/*
 * Bootloader.c
 *
 *  Created on: Jan 24, 2024
 *      Author: Hind eldeghaidy
 */

/*includes*/
#include "STD_TYPES.h"
#include "Bootloader.h"

/*Static Function Declaration*/
static u8 Bootloader_CRC_Verify(u8 *pData ,u32 Data_Length,u32 Host_CRC);
//static void Bootloader_Get_Version(u8 *Host_Buffer);
//static void Bootloader_Get_Help(u8 *Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(u8 *Host_Buffer);
//static void Bootloader_Read_Protection_Level(u8 *Host_Buffer);
//static void Bootloader_Jump_To_Address(u8 *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(u8 *Host_Buffer);
/*static void Bootloader_Enable_RW_Protection(u8 *Host_Buffer);
static void Bootloader_Memory_Read(u8 *Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status(u8 *Host_Buffer);
static void Bootloader_Read_OTP(u8 *Host_Buffer);
 */
//static void Bootloader_Disable_RW_Protection(u8 *Host_Buffer);
void Bootloader_Jump_To_User_App(void);
//static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer);


static void Bootloader_send_Ack(u8 Send_u8ReplayLen);
static void Bootloader_send_NAck(void);

static void Bootloader_Send_Data_To_Host(u8 *Host_Buffer,u32 Data_Len);
/*Global Variable Definitions*/
static u8 BL_HOST_BUFFER[BL_HOST_BUFFER_RX_LENGTH];
static u8 Bootloader_Supported_CMDs[12]=
{
		//CBL_Jump_TO_APP,
		CBL_GET_VER_CMD	,
		CBL_GET_HELP_CMD,
		CBL_GET_CID_CMD	,
		CBL_GET_RDP_STATUS_CMD,
		CBL_GO_TO_ADDR_CMD,
		CBL_FLASH_ERASE_CMD,
		CBL_MEM_WRITE_CMD,
		CBL_ED_W_PROTECT_CMD,
		CBL_MEM_READ_CMD,
		CBL_READ_SECTOR_STATUS_CMD,
		CBL_OTP_READ_CMD,
		CBL_CHANGE_ROP_Level_CMD
};
/*Software Interface Definitions*/
BL_Status BL_UART_Fetch_Host_Command(void)
{
	BL_Status status=BL_NACK;
	HAL_StatusTypeDef HAL_status=HAL_ERROR;
	u8 Data_Length=0;
	memset(BL_HOST_BUFFER,0,BL_HOST_BUFFER_RX_LENGTH);
	HAL_status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, BL_HOST_BUFFER, 1, 1000);
	if(HAL_status!=HAL_OK)
	{
		status=BL_NACK;
	}
	else
	{
		Data_Length=BL_HOST_BUFFER[0];
		HAL_status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &BL_HOST_BUFFER[1],Data_Length, 1000);
		if(HAL_status!=HAL_OK)
		{
			status=BL_NACK;
		}
		else
		{
			switch(BL_HOST_BUFFER[1])
			{
			/*case CBL_Jump_TO_APP:
				Bootloader_Jump_To_User_App();
				status=BL_OK;
				break;*/
			/*case CBL_GET_VER_CMD:
				Bootloader_Get_Version(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_GET_HELP_CMD:
				Bootloader_Get_Help(BL_HOST_BUFFER);
				status=BL_OK;
				break;*/
			case CBL_GET_CID_CMD:
				Bootloader_Get_Chip_Identification_Number(BL_HOST_BUFFER);
				status=BL_OK;
				break;
				/*case CBL_GET_RDP_STATUS_CMD:
				Bootloader_Read_Protection_Level(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_GO_TO_ADDR_CMD:
				Bootloader_Jump_To_Address(BL_HOST_BUFFER);
				status=BL_OK;
				break;*/
			case CBL_FLASH_ERASE_CMD:

				Bootloader_Erase_Flash(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_MEM_WRITE_CMD:
				Bootloader_Memory_Write(BL_HOST_BUFFER);
				status=BL_OK;
				break;
				/*case CBL_ED_W_PROTECT_CMD:
				BL_Print_Message("Enable Read/Write Protect On Different Sector Of the User Flash\r\n");
				Bootloader_Enable_RW_Protection(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_MEM_READ_CMD:
				BL_Print_Message("Read Data From Different Memories Of the MicroController\r\n");
				Bootloader_Memory_Read(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_READ_SECTOR_STATUS_CMD:
				BL_Print_Message("Read All The Sector Protection Status\r\n");
				Bootloader_Get_Sector_Protection_Status(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_OTP_READ_CMD:
				BL_Print_Message("Read The OTP Contents\r\n");
				Bootloader_Read_OTP(BL_HOST_BUFFER);
				status=BL_OK;
				break;
			case CBL_CHANGE_ROP_Level_CMD:
				Bootloader_Change_Read_Protection_Level(BL_HOST_BUFFER);
				status=BL_OK;
				break;
				 */
			default:
				BL_Print_Message("Invalid Command Code Received From Host!!\r\n");
				status=BL_OK;
				break;
			}
		}
	}
	return status;
}
/*Static function Definition*/
void Bootloader_Jump_To_User_App(void)
{
	/*value of the main Stack Pointer of our main Application*/
	//u32 MSP_Value =*((volatile u32*)(FLASH_SECTOR2_BASE_ADDRESS));
	/*Reset Handler Definition Function of Our Main Application*/
	u32 MainAppAddr =*((volatile u32*)(FLASH_SECTOR2_BASE_ADDRESS+4));
	/* Fetch the reset handler address of the user application */
	pMainApp ResetHandler_Address = (pMainApp)MainAppAddr;

	/*Set Main Stack Pointer*/
	//__set_MSP(MSP_Value);
	/*DeInitialize of Modules*/
	HAL_RCC_DeInit();/*Resets the RCC clock configuration to the default reset state.*/
	/*Jump To Application Reset Handler*/
	ResetHandler_Address();

}
static u8 Bootloader_CRC_Verify(u8 *pData ,u32 Data_Length,u32 Host_CRC)
{
	u8 Local_CRC_Status=CRC_VERIFICATION_FAILED;
	u32 MCU_CRC_Calculated = 0;
	u8 data_counter=0;
	u32 Data_Buffer=0;
	/*Calculate CRC32*/
	for(data_counter = 0;data_counter<Data_Length;data_counter++)
	{
		Data_Buffer=(u32)pData[data_counter];
		MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE, &Data_Buffer, 1);
	}
	/*Reset CRC Calculation Unit*/
	__HAL_CRC_DR_RESET(CRC_ENGINE);
	/*Compare the HOST CRC To The Calculated one*/
	if(MCU_CRC_Calculated == Host_CRC)
	{
		Local_CRC_Status=CRC_VERIFICATION_APPROVED;
	}
	else
	{
		Local_CRC_Status=CRC_VERIFICATION_FAILED;
	}
	return Local_CRC_Status;
}
static void Bootloader_send_Ack(u8 Send_u8ReplayLen)
{
	u8 ACK_Value[2]={0};
	ACK_Value[0]= CBL_SEND_ACK;
	ACK_Value[1]= Send_u8ReplayLen;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,(u8 *)ACK_Value,2, 1000);
}
static void Bootloader_send_NAck(void)
{
	u8 ACK_Value=CBL_SEND_NACK;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,&ACK_Value,1, 1000);
}
static void Bootloader_Send_Data_To_Host(u8 *Host_Buffer,u32 Data_Len)
{
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, Data_Len, 1000);
}
/*
static void Bootloader_Get_Version(u8 *Host_Buffer)
{
	u8 BL_VERSION[4]={CBL_VENDOR_ID,CBL_SW_MAJOR_VERSION,CBL_SW_MINOR_VERSION,CBL_SW_PATCH_VERSION};
	u16 HOST_CMD_PACKET_LENGTH=0;
	u32 HOST_CRC32=0; //32bit 4byte to store CRC
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
	BL_Print_Message("Read The BootLoader Version from the MCU\r\n");
#endif
	EXTRACT THE CRC32 AND PACKET LENGTH SENT BY THE HOST
	HOST_CMD_PACKET_LENGTH=Host_Buffer[0]+1;
	HOST_CRC32= *((u32 *)((Host_Buffer+HOST_CMD_PACKET_LENGTH)-CRC_TYPE_SIZE_BYTE));
	CRC Verification
	if (CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8 *)&Host_Buffer[0], HOST_CMD_PACKET_LENGTH-4, HOST_CRC32))
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed\r\n");
#endif
		Bootloader_send_Ack(4);
		Bootloader_Send_Data_To_Host((u8 *)&BL_VERSION[0],4);
		//Bootloader_Jump_To_User_App();
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("Bootloader Ver. %d%d%d \r\n",BL_VERSION[1],BL_VERSION[2],BL_VERSION[3]);

#endif
	}
	else
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed\r\n");
#endif
		Bootloader_send_NAck();
	}
}
static void Bootloader_Get_Help(u8 *Host_Buffer)
{
	u16 HOST_CMD_PACKET_LENGTH=0;
	u32 HOST_CRC32=0; //32bit 4byte to store CRC
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
	BL_Print_Message("Read The Commands Supported By the BootLoader\r\n");
#endif
	EXTRACT THE CRC32 AND PACKET LENGTH SENT BY THE HOST
	HOST_CMD_PACKET_LENGTH = Host_Buffer[0] + 1;
	HOST_CRC32=*((u32 *)((Host_Buffer+HOST_CMD_PACKET_LENGTH)-CRC_TYPE_SIZE_BYTE));
	CRC Verification
	if (CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8*)&Host_Buffer[0], HOST_CMD_PACKET_LENGTH-CRC_TYPE_SIZE_BYTE, HOST_CRC32))
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed\r\n");
#endif
		Bootloader_send_Ack(12);
		Bootloader_Send_Data_To_Host((u8*)(&Bootloader_Supported_CMDs[0]),12);

	}
	else
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed\r\n");
#endif
		Bootloader_send_NAck();
	}
}
 */
static void Bootloader_Get_Chip_Identification_Number(u8 *Host_Buffer)
{
	u16 HOST_CMD_PACKET_LENGTH=0;
	u32 HOST_CRC32=0; //32bit 4byte to store CRC
	u16 MCU_Identification_Number=0;
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
	BL_Print_Message("Read The MCU Chip Identification Number\r\n");
#endif
	/*EXTRACT THE CRC32 AND PACKET LENGTH SENT BY THE HOST*/
	HOST_CMD_PACKET_LENGTH=Host_Buffer[0]+1;
	HOST_CRC32=*((u32 *)((Host_Buffer+HOST_CMD_PACKET_LENGTH)-CRC_TYPE_SIZE_BYTE));
	/*CRC Verification*/
	if (CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8*)&Host_Buffer[0], HOST_CMD_PACKET_LENGTH-4, HOST_CRC32))
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed\r\n");
#endif
		/*GET The MCU Chip Identification Number*/
		/*Report The MCU Chip Identification Number to host */
		MCU_Identification_Number =(u16)(DBGMCU->IDCODE)&0x00000FFF;
		Bootloader_send_Ack(2);
		Bootloader_Send_Data_To_Host((u8*)&MCU_Identification_Number,2);
		Bootloader_Jump_To_User_App();
	}
	else
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed\r\n");
#endif
		Bootloader_send_NAck();
	}
}
/*
static u8 CBL_STM32F401_Get_RDP_Level(void)
{
	FLASH_OBProgramInitTypeDef FLASH_OBProgram ;
	 Get the Option byte configuration
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
	return (u8)(FLASH_OBProgram.RDPLevel);

}
static void Bootloader_Read_Protection_Level(u8 *Host_Buffer)
{
	u16 Host_CMD_Packet_Len = 0;
	u32 Host_CRC32 = 0;
	u8 RDP_Level=0;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read The Flash Read Protection Level\r\n");
#endif
	 Extract the CRC32 and packet length sent by the HOST
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32=*((u32 *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE));
	 CRC Verification
	if(CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8 *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		 Send acknowledgment to the HOST
		Bootloader_send_Ack(1);
		Reqyest To Change The Read Out Protection Level
		RDP_Level = CBL_STM32F401_Get_RDP_Level();
		Report The Read Protection Level to host
		Bootloader_Send_Data_To_Host((u8*)&RDP_Level,1);

	}
	else
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed\r\n");
#endif
		Bootloader_send_NAck();
	}
}

 */
static uint8_t Host_Address_Verification(uint32_t Jump_Address)
{
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	if((Jump_Address >= SRAM1_BASE) && (Jump_Address <= STM32F401_FLASH_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else if((Jump_Address >= FLASH_BASE) && (Jump_Address <= STM32F401_FLASH_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}
/*
static void Bootloader_Jump_To_Address(u8 *Host_Buffer)
{
	u16 Host_CMD_Packet_Len = 0;
	u32 Host_CRC32 = 0;
	u32 HOST_Jump_Address = 0;
	u8 Address_Verification = ADDRESS_IS_INVALID;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Jump Bootloader to specified address \r\n");
#endif
	 Extract the CRC32 and packet length sent by the HOST
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32=*((u32 *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE));
	 CRC Verification
	if(CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_send_Ack(1);
		 Extract the address form the HOST packet
		HOST_Jump_Address = *((uint32_t *)&Host_Buffer[2]);
		 Verify the Extracted address to be valid address
		Address_Verification = Host_Address_Verification(HOST_Jump_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Address verification succeeded \r\n");
#endif
			 Report address verification succeeded
			Bootloader_Send_Data_To_Host((u8 *)&Address_Verification, 1);
			 Prepare the address to jump
			Jump_Ptr Jump_Address = (Jump_Ptr)(HOST_Jump_Address + 1);//t pit
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Jump to : 0x%X \r\n", Jump_Address);
#endif
			Jump_Address();
		}
		else{
			 Report address verification failed
			Bootloader_Send_Data_To_Host((u8 *)&Address_Verification, 1);
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_send_NAck();
	}

}
 */
static u8 Perform_Flash_Erase(u8 Sector_Number,u8 Number_Of_Sectors)
{
	u8 Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	FLASH_EraseInitTypeDef pEraseInit;
	u8 Remaining_Sectors = 0;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	u32 SectorError = 0;

	if(Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER){
		/* Number Of sectors is out of range */
		Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	}
	else{
		if((Sector_Number <= (CBL_FLASH_MAX_SECTOR_NUMBER - 1)) || (CBL_FLASH_MASS_ERASE == Sector_Number))
		{
			/* Check if user needs Mass erase */
			if(CBL_FLASH_MASS_ERASE == Sector_Number)
			{
				pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE; /* Flash Mass erase activation */
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Flash Mass erase activation \r\n");
#endif
			}
			else
			{
				/* User needs Sector erase */
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("User needs Sector erase \r\n");
#endif
				Remaining_Sectors = CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Number;
				//Sector_Number =4 , Number_Of_Sector=6   4->10
				//i have only 0->7 Sector Remaining_Sectors =8-4=4 (6>4) Number_Of_Sector=4
				if(Number_Of_Sectors > Remaining_Sectors)
				{
					Number_Of_Sectors = Remaining_Sectors;
				}
				else { /* Nothing */ }

				pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS; /* Sectors erase only */
				pEraseInit.Sector = Sector_Number;        /* Initial FLASH sector to erase when Mass erase is disabled */
				pEraseInit.NbSectors = Number_Of_Sectors; /* Number of sectors to be erased. */
			}

			pEraseInit.Banks = FLASH_BANK_1; /* Bank 1  */
			pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; /* Device operating range: 2.7V to 3.6V */

			/*Unlock Flash Control Register Access*/
			HAL_Status = HAL_FLASH_Unlock();
			if(HAL_Status != HAL_OK)
			{
				Sector_Validity_Status=UNSUCCESSFUL_ERASE;;
			}
			else{

				/* Perform a mass erase or erase the specified FLASH memory sectors */
				HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
				//(0xFFFFFFFFU means that all the sectors have been correctly erased)
				if(SectorError == HAL_SUCCESSFUL_ERASE)
				{
					Sector_Validity_Status=SUCCESSFUL_ERASE;
				}
				else
				{
					Sector_Validity_Status=UNSUCCESSFUL_ERASE;
				}
				/*Lock Flash Control Register Access*/
				HAL_Status =HAL_FLASH_Lock();
			}
		}
		else
		{
			Sector_Validity_Status=UNSUCCESSFUL_ERASE;
		}

	}
	return Sector_Validity_Status;
}
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer){
	u16 Host_CMD_Packet_Len = 0;
	u32 Host_CRC32 = 0;
	u8 Erase_Status = 0;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Mass erase or sector erase of the user flash \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32=*((u32 *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8 *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgment to the HOST */
		Bootloader_send_Ack(1);
		/* Perform Mass erase or sector erase of the user flash */
		Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		if(SUCCESSFUL_ERASE == Erase_Status){
			/* Report erase Passed */
			Bootloader_Send_Data_To_Host((u8 *)&Erase_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Successful Erase \r\n");
#endif
		}
		else{
			/* Report erase failed */
			Bootloader_Send_Data_To_Host((u8 *)&Erase_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Erase request failed !!\r\n");
#endif
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_send_NAck();
	}
}
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len){
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_Counter = 0;

	/* Unlock the FLASH control register access */
	HAL_Status = HAL_FLASH_Unlock();

	if(HAL_Status != HAL_OK){
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else{
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++){
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Payload_Start_Address + Payload_Counter, Host_Payload[Payload_Counter]);
			if(HAL_Status != HAL_OK){
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
				break;
			}
			else{
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
			}
		}
	}

	if((FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) && (HAL_OK == HAL_Status)){
		/* Locks the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock();
		if(HAL_Status != HAL_OK){
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
		}
		else{
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
		}
	}
	else{
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}

	return Flash_Payload_Write_Status;
}

static void Bootloader_Memory_Write(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint32_t HOST_Address = 0;
	uint8_t Payload_Len = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Write data into different memories of the MCU \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_send_Ack(1);
		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("HOST_Address = 0x%X \r\n", HOST_Address);
#endif
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[6];
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(HOST_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
			/* Write the payload to the Flash memory */
			Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);
			if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status){
				/* Report payload write passed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload Valid \r\n");
#endif
			}
			else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload InValid \r\n");
#endif
				/* Report payload write failed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);


			}
		}
		else{
			/* Report address verification failed */
			Address_Verification = ADDRESS_IS_INVALID;
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_send_NAck();
	}
}
/*
static void Bootloader_Enable_RW_Protection(u8 *Host_Buffer)
{

}
static void Bootloader_Memory_Read(u8 *Host_Buffer)
{

}

static u8 Change_ROP_Level(u32 ROP_Level)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	FLASH_OBProgramInitTypeDef FLASH_OBProgramInit;
	u8 ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;

	 Unlock the FLASH Option Control Registers access
	HAL_Status = HAL_FLASH_OB_Unlock();
	if(HAL_Status != HAL_OK){
		ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Failed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Passed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
		FLASH_OBProgramInit.OptionType = OPTIONBYTE_RDP;  RDP option byte configuration
		FLASH_OBProgramInit.Banks = FLASH_BANK_1;
		FLASH_OBProgramInit.RDPLevel = ROP_Level;
		 Program option bytes
		HAL_Status = HAL_FLASHEx_OBProgram(&FLASH_OBProgramInit);
		if(HAL_Status != HAL_OK){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Failed -> Program option bytes \r\n");
#endif
			HAL_Status = HAL_FLASH_OB_Lock();
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
		}
		else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Passed -> Program option bytes \r\n");
#endif
			 Launch the option byte loading
			HAL_Status = HAL_FLASH_OB_Launch();
			if(HAL_Status != HAL_OK){
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
			}
			else{
				 Lock the FLASH Option Control Registers access
				HAL_Status = HAL_FLASH_OB_Lock();
				if(HAL_Status != HAL_OK){
					ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
				}
				else{
					ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
					BL_Print_Message("Passed -> Program ROP to Level : 0x%X \r\n", ROP_Level);
#endif
				}
			}
		}
	}
	return ROP_Level_Status;
}
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer)
{
	u16 Host_CMD_Packet_Len = 0;
	u32 HOST_CRC32 = 0;
	u8 ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
	u8 Host_ROP_Level=0;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Change Read Protection Level On User Flash\r\n");
#endif
	 Extract the CRC32 and packet length sent by the HOST
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	HOST_CRC32=*((u32 *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE));
	 CRC Verification
	if(CRC_VERIFICATION_APPROVED == Bootloader_CRC_Verify((u8 *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, HOST_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		 Send acknowledgment to the HOST
		Bootloader_send_Ack(1);
		Host_ROP_Level=Host_Buffer[2];
		if(Host_ROP_Level == 2)
		{
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
		}
		else
		{
			if(Host_ROP_Level ==1)
			{
				Host_ROP_Level =0xAA;
			}
			else if(Host_ROP_Level ==0)
			{
				Host_ROP_Level =0x55 ;
			}
			ROP_Level_Status = Change_ROP_Level((u32)Host_Buffer[2]);
		}
		Bootloader_Send_Data_To_Host((u8 *)&ROP_Level_Status, 1);
	}
	else
	{
#if(BL_DEBUG_ENABLE==DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed\r\n");
#endif
		Bootloader_send_NAck();
	}

}

static void Bootloader_Get_Sector_Protection_Status(u8 *Host_Buffer)
{

}
static void Bootloader_Read_OTP(u8 *Host_Buffer)
{

}
//static void Bootloader_Disable_RW_Protection(u8 *Host_Buffer){}
 */
void BL_Print_Message(char *format,...)
{
	char Message[100]={0};
	//Hold the information needed by va_start,va_end,va_arg
	va_list args;
	//Enables access to the Variable arguments
	va_start(args,format);
	//Write Formatted data from variable argument list to string
	vsprintf(Message,format,args);
#if (BL_DEBUG_METHOD==BL_ENABLE_UART_DEBUG_MESSAGE)
	//Transmit the Formatted data through the defined UART
	HAL_UART_Transmit(BL_DEBUG_UART, (u8*)Message, sizeof(Message), 1000);
#elif (BL_DEBUG_METHOD==BL_ENABLE_SPI_DEBUG_MESSAGE)
	//Transmit the Formatted data through the defined SPI

#elif (BL_DEBUG_METHOD==BL_ENABLE_CAN_DEBUG_MESSAGE)
	//Transmit the Formatted data through the defined CAN

#else
#error "Wrong BL Configuration Method"
#endif
	//Perform Cleanup for app object initalized by a call to va_start
	va_end(args);
}

