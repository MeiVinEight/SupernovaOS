//
// Created by MeiVi on 2026/04/08.
//

#pragma once

#include <types.h>

#define CDB_CMD_TEST_UNIT_READY   0x00
#define CDB_CMD_REQUEST_SENSE     0x03
#define CDB_CMD_INQUIRY           0x12
#define CDB_CMD_READ_16           0x88
#define CDB_CMD_WRITE_16          0x8A
#define CDB_CMD_SERVICE_ACTION_IN 0x9E


#define CDB_CMD_SAI_READ_CAPACITY_16 0x10

typedef struct _SCSI_INQUIRY_COMMAND
{
	BYTE CODE;
	BYTE EVPD:1;
	BYTE RSV0:4;
	BYTE LUNN:3;
	BYTE PAGE;
	BYTE RSV1;
	BYTE ALEN;
	BYTE RSV2[7];
} SCSI_COMMAND_INQUIRY;
typedef struct _SCSI_INQUIRY_DATA
{
	BYTE PDTY:5;
	BYTE RSV0:3;

	BYTE RSV1:7;
	BYTE RMOV:1;

	BYTE ANSV:3;
	BYTE ECMV:3;
	BYTE ISOV:2;

	BYTE RDFM:4;
	BYTE RSV2:4;

	BYTE ALEN;
	BYTE RSV3[3];
	BYTE VEND[8];
	BYTE PROD[16];
	BYTE REVI[4];
} SCSI_DATA_INQUIRY;
#pragma pack(push, 1)
typedef struct _SCSI_READ_CAPACITY16_COMMAND
{
	/**
	 * OPERATION CODE field
	 * The OPERATION CODE field contains the operation code of a command supported by the logical unit and shall be set to the value defined in
	 * table 32.
	 */
	BYTE  CODE;
	// The SERVICE ACTION field is defined in defined in 2.2.2 and set to the value defined in table 32.
	BYTE  SACT:5;
	BYTE  RSV0:3;
	/**
	 * This field has been declared obsolete by the T10 committee. However, it is included because it may be implemented on some products.
	 *
	 * The LOGICAL BLOCK ADDRESS field shall be set to zero if the PMI bit is set to zero. If the PMI bit is set to zero and the LOGICAL BLOCK ADDRESS
	 * field is not set to zero, the device server shall terminate the command with CHECK CONDITION status with the sense key set to ILLEGAL
	 * REQUEST and the additional sense code set to INVALID FIELD IN CDB.
	 */
	QWORD LBAX;
	BYTE  RSV2[3];
	/**
	 * The ALLOCATION LENGTH field (see 2.2.6) specifies the maximum number of bytes that the application client has allocated for returned parameter data.
	 *
	 * An allocation length of zero indicates that no data shall be transferred. This condition shall not be considered as an error. The device
	 * server shall terminate transfers to the data-in buffer when the number of bytes specified by the ALLOCATION LENGTH field have been transferred
	 * or when all available data has been transferred, whichever is less. The contents of the parameter data shall not be altered to reflect the
	 * truncation, if any, that results from an insufficient allocation length.
	 */
	BYTE  ALEN;
	/**
	 * PMI (Partial Medium Indicator) bit
	 * This bit has been declared obsolete by the T10 committee. However, it is included because it may be implemented on some products.
	 *  - 0 A partial medium indicator (PMI) bit set to zero specifies that the device server return information on the last logical block on the
	 * direct-access block device.
	 *  - 1 A PMI bit set to one specifies that the device server return information on the last logical block after that specified in the LOGICAL BLOCK
	 * ADDRESS field before a substantial vendor-specific delay in data transfer may be encountered.
	 *
	 * This function is intended to assist storage management software in determining whether there is sufficient space starting with the logical
	 * block address specified in the CDB to contain a frequently accessed data structure (e.g., a file directory or file index) without incurring an extra
	 * delay.
	 */
	BYTE  PMIB:1;
	BYTE  RSV1:7;
	BYTE  CTRL;
} SCSI_READ_CAPACITY16_COMMAND;
typedef struct _SCSI_READ_CAPACITY16_DATA
{
	/**
	 * RETURNED LOGICAL BLOCK ADDRESS field
	 *
	 * The RETURNED LOGICAL BLOCK ADDRESS field and LOGICAL BLOCK LENGTH IN BYTES field of the READ CAPACITY (16) parameter data are the
	 * same as the in the READ CAPACITY (10) parameter data (see 3.24.1). The maximum value that shall be returned in the RETURNED LOGICAL
	 * BLOCK ADDRESS field is FFFFFFFF_FFFFFFFEh.
	 */
	QWORD LBAX;
	/**
	 * LOGICAL BLOCK LENGTH IN BYTES field
	 *
	 * The LOGICAL BLOCK LENGTH IN BYTES field contains the number of bytes of user data in the logical block indicated by the RETURNED LOGICAL
	 * BLOCK ADDRESS field. This value does not include protection information or additional information (e.g., ECC bytes) recorded on the medium.
	 */
	DWORD LBLX;
	// P_TYPE (Protection Type) field and PROT_EN bit
	// The protection type (P_TYPE) field and the PROT_EN bit (see table 136) indicate the logical unit’s current type of protection.
	BYTE  POEN:1;
	// P_TYPE (Protection Type) field and PROT_EN bit
	// The protection type (P_TYPE) field and the PROT_EN bit (see table 136) indicate the logical unit’s current type of protection.
	BYTE  POTY:3;
	/**
	 * RC BASIS field
	 *
	 * The RC BASIS field indicates the meaning of the value returned by the RETURNED LOGICAL BLOCK ADDRESS field in the READ CAPACITY (16)
	 * parameter data and is described in table 135.
	 *
	 * Table 135 RC BASIS field
	 * - 00b The RETURNED LOGICAL BLOCK ADDRESS field indicates the highest LBA of a contiguous
	 * range of zones that are not sequential write required zones starting with the first zone.
	 * - 01b The RETURNED LOGICAL BLOCK ADDRESS field indicates the LBA of the last logical block
	 * on the logical unit.
	 * - 10b Reserved
	 * - 11 Reserved
	 */
	BYTE  RCBA:2;
	BYTE  RSV0:2;
	/**
	 * LOGICAL BLOCKS PER PHYSICAL BLOCKS EXPONENT field
	 *
	 * The LOGICAL BLOCKS PER PHYSICAL BLOCKS EXPONENT field is defined in table 137.
	 *
	 * - 0: One or more physical blocks per logical block[a]
	 * - n > 0: 2^n logical blocks per physical block
	 */
	BYTE  PBEX:4;
	/**
	 * P_I_EXPONENT field
	 *
	 * The P_I_EXPONENT field may be used to determine the number of protection information intervals placed within each logical block (see 3.3.2).
	 *
	 * The number of protection information intervals is calculated as follows:
	 * number of protection information intervals = 2^(p_i exponent)
	 * where: p_i exponent is the contents of the P_I EXPONENT field
	 */
	BYTE  PIEX:4;
	union
	{
		/**
		 * LOWEST ALIGNED LOGICAL BLOCK ADDRESS field
		 * The LOWEST ALIGNED LOGICAL BLOCK ADDRESS field indicates the LBA of the first logical block that is located at the beginning of a physical
		 * block (SBC-5).
		 */
		WORD  LBAL;
		struct
		{
			BYTE RSV1:6;
			/**
			 * Logical Block Provisioning Read Zeros (LBPRZ) bit
			 *
			 * - 1 If the logical block provisioning read zeros (LBPRZ) bit is set to one, then, for an unmapped LBA specified by a read operation, the device
			 * server shall send user data with all bits set to zero to the data-in buffer.
			 * - 0 If the TPRZ bit is set to zero, then, for an unmapped LBA specified by a read operation, the device server may send user data with all bits
			 * set to any value to the data-in buffer.
			 */
			BYTE BPRZ:1;
			/**
			 * Logical Block Provisioning Management Enabled (LBPME) bit
			 *
			 * - 1 If the thin provisioning enabled (TPE) bit is set to one, then the logical unit implements logical block provisioning management (see
			 * SBC-5).
			 * - 0 If the TPE bit is set to zero, then the logical unit does not implement logical block provisioning management (see SBC-5)
			 */
			BYTE BPME:1;
			BYTE RSV2;
		};
	};
} SCSI_READ_CAPACITY16_DATA;
typedef struct _SCSI_READ16_COMMAND
{
	BYTE  CODE;
	/**
	 * DLD2, DLD1 and DLD0 bits
	 *
	 * The command duration time (see SAM-6) is specified by the command duration limit descriptor (see 5.3.10 and 5.3.11) specified by the DLD2
	 * bit, the DLD1 bit, and the DLD0 bit, as shown in table 113. The CDLP field in the REPORT SUPPORTED OPERATION CODES parameter data (see
	 * SPC-6) indicates that the command duration limit descriptor is in the Command Duration Limit A mode page or the Command Duration Limit
	 * B mode page (see 5.3.10 and 5.3.11).
	 */
	BYTE  DLD2:1;
	BYTE  OBSO:1;
	/**
	 * RARC bit
	 *
	 * If rebuild assist mode is supported and not enabled, then the rebuild assist recovery control (RARC) bit is ignored.
	 *
	 * If rebuild assist mode is supported and enabled, then the RARC bit specifies that read operations are processed as defined in SBC-5.
	 *
	 * If the rebuild assist mode is not supported and the RARC bit is set to one, then the device server should terminate the command with CHECK
	 * CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set to INVALID FIELD IN CDB.
	 */
	BYTE  RARC:1;
	/**
	 * FUA bit
	 *
	 * - 0 A Force unit Access (FUA) bit set to zero specifies that the device server may read the logical blocks from the volatile cache (if any), the
	 * specified data pattern for that LBA (e.g., the data pattern for unmapped data), the non-volatile cache, or the medium.
	 * - 1 An FUA bit set to one specifies that the device server shall read the logical blocks from the specified data pattern for that LBA, the
	 * non-volatile cache (if any), or the medium. If a volatile cache contains a more recent version of a logical block, then the device server
	 * shall write that logical block to non-volatile cache or the medium before reading the logical block.
	 */
	BYTE  FUAC:1;
	/**
	 * DPO (Disable Page Out) bit
	 *
	 * - 0 A Disable Page Out (DPO) bit set to zero specifies that the retention priority shall be determined by the RETENTION PRIORITY fields in the
	 * Caching mode page (see 5.3.9).
	 * - 1 A DPO bit set to one specifies that the device server shall assign the logical blocks accessed by this command the lowest retention priority for being fetched into or retained by the cache. A DPO bit set to one overrides any retention priority specified in the Caching mode
	 * page. All other aspects of the algorithm implementing the cache replacement strategy are not defined by this manual.
	 *
	 * NOTE The DPO bit is used to control replacement of logical blocks in the cache when the application client
	 * has information on the future usage of the logical blocks. If the DPO bit is set to one, the application
	 * client is specifying that the logical blocks accessed by the command are not likely to be accessed
	 * again in the near future and should not be put in the cache nor retained by the cache. If the DPO bit
	 * is set to zero, the application client is specifying that the logical blocks accessed by this command are
	 * likely to be accessed again in the near future.
	 */
	BYTE  DPOU:1;
	/**
	 * RDPROTECT field
	 *
	 * The device server shall check the protection information read from the medium before returning status for the command based on the RDPROTECT field as described in table 110.
	 */
	BYTE  RDPO:3;
	/**
	 * LOGICAL BLOCK ADDRESS field
	 *
	 * The LOGICAL BLOCK ADDRESS field specifies the first logical block accessed by this command. If the logical block address exceeds the capacity
	 * of the medium the device server shall terminate the command with CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and
	 * the additional sense code set to LOGICAL BLOCK ADDRESS OUT OF RANGE.
	 */
	QWORD LBAX;
	/**
	 * TRANSFER LENGTH field
	 *
	 * The TRANSFER LENGTH field specifies the number of contiguous logical blocks of data that shall be read and transferred to the data-in buffer,
	 * starting with the logical block specified by the LOGICAL BLOCK ADDRESS field. A TRANSFER LENGTH field set to zero specifies that no logical
	 * blocks shall be read. This condition shall not be considered an error. Any other value specifies the number of logical blocks that shall be read. If
	 * the logical block address plus the transfer length exceeds the capacity of the medium, the device server shall terminate the command with
	 * CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set to LOGICAL BLOCK ADDRESS OUT OF
	 * RANGE. The TRANSFER LENGTH field is constrained by the MAXIMUM TRANSFER LENGTH field in the Block Limits VPD page.
	 */
	DWORD TLEN;
	/**
	 * GROUP NUMBER field
	 *
	 * The GROUP NUMBER field specifies the group into which attributes associated with the command should be collected. A GROUP NUMBER field
	 * set to zero specifies that any attributes associated with the command shall not be collected into any group.
	 */
	BYTE  GNUM:6;
	BYTE  DLD0:1;
	BYTE  DLD1:1;
	BYTE  CTRL;
} SCSI_READ16_COMMAND;
#pragma pack(pop)