#pragma once

#include <types.h>
#include <driver/pci/pcie.h>

typedef struct _NVM_EXPRESS_CAPABILITY
{
	/**
	 * Maximum Queue Entries Supported (MQES): This field indicates the maximum
	 * individual queue size that the controller supports. For NVMe over PCIe
	 * implementations, this value applies to the I/O Submission Queues and I/O
	 * Completion Queues that the host creates. For NVMe over Fabrics implementations,
	 * this value applies to only the I/O Submission Queues that the host creates. This is
	 * a 0’s based value. The minimum value is 1h, indicating two entries.
	 */
	QWORD MQES:16;
	/**
	 * Contiguous Queues Required (CQR): This bit is set to ‘1’ if the controller requires
	 * that I/O Submission Queues and I/O Completion Queues are required to be
	 * physically contiguous. This bit is cleared to ‘0’ if the controller supports I/O
	 * Submission Queues and I/O Completion Queues that are not physically
	 * contiguous. If this bit is set to ‘1’, then the Physically Contiguous bit (CDW11.PC)
	 * in the Create I/O Submission Queue and Create I/O Completion Queue commands
	 * shall be set to ‘1’.
	 *
	 * For controllers using a message-based transport, this property shall be set to a
	 * value of 1.
	 */
	QWORD CQRR:1;
	/**
	 * Arbitration Mechanism Supported (AMS) - RO: This field is bit significant and
	 * indicates the optional arbitration mechanisms supported by the controller. If a bit is
	 * set to ‘1’, then the corresponding arbitration mechanism is supported by the
	 * controller. Refer to section 3.4.4 for arbitration details.
	 * - 1 Vendor Specific (VS): Vendor Specific arbitration mechanism.
	 * - 0
	 * Weighted Round Robin with Urgent Priority Class (WRRUPC):
	 * Weighted Round Robin with Urgent Priority Class arbitration mechanism.
	 *
	 * The round robin arbitration mechanism is not listed since all controllers shall
	 * support this arbitration mechanism.
	 *
	 * For Discovery controllers, this property shall be cleared to 0h.
	 */
	QWORD AMSU:2;
	QWORD RSV1:5;
	/**
	 * Timeout (TO) - RO: This is the worst-case time that a host should wait for the
	 * CSTS.RDY bit to transition from:
	 * - a) ‘0’ to ‘1’ after the CC.EN bit transitions from ‘0’ to ‘1’; or
	 * - b) ‘1’ to ‘0’ after the CC.EN bit transitions from ‘1’ to ‘0’.
	 *
	 * This worst-case time may be experienced after events such as an abrupt shutdown,
	 * loss of main power without shutting down the controller, or activation of a new
	 * firmware image; typical times are expected to be much shorter.
	 *
	 * This field is in 500 millisecond units. The maximum value of this field is FFh, which
	 * indicates a 127.5 second timeout.
	 *
	 * If the Controller Ready Independent of Media Enable (CC.CRIME) bit is cleared to
	 * ‘0’ and the worst-case time for the CSTS.RDY bit to change state is due to enabling
	 * the controller after the CC.EN bit transitions from ‘0’ to ‘1’, then this field shall be
	 * set to:
	 * - a) the value in the Controller Ready With Media Timeout (CRTO.CRWMT)
	 * field; or
	 * - b) FFh if the value in the CRTO.CRWMT field is greater than FFh.
	 *
	 * If the Controller Ready Independent of Media Enable (CC.CRIME) bit is set to ‘1’
	 * and the worst-case time for the CSTS.RDY bit to change state is due to enabling
	 * the controller after the CC.EN bit transitions from ‘0’ to ‘1’, then this field shall be
	 * set to:
	 * - a) the value in the Controller Ready Independent of Media Timeout
	 * (CRTO.CRIMT); or
	 * - b) FFh if the value in the CRTO.CRIMT field is greater than FFh.
	 *
	 * Controllers that support the CRTO property (refer to Figure 57) are able to indicate
	 * larger timeouts for enabling the controller. A host should use the value in the
	 * CRTO.CRWMT field or the CRTO.CRIMT field depending on the controller ready
	 * mode indicated by the CC.CRIME bit to determine the worst-case timeout for the
	 * CSTS.RDY bit to transition from ‘0’ to ‘1’ after the CC.EN bit transitions from ‘0’ to
	 * ‘1’. A host that is based on revisions earlier than NVM Express Base Specification,
	 * Revision 2.0 is not required to wait for more than 127.5 seconds for the CSTS.RDY
	 * bit to transition.
	 *
	 * Refer to sections 3.5.3 and 3.5.4 for more information.
	 */
	QWORD TIMO:8;
	/**
	 * Doorbell Stride (DSTRD): Each Submission Queue and Completion Queue
	 * Doorbell property is 32-bits in size. This property indicates the stride between
	 * doorbell properties. The stride is specified as (2 ^ (2 + DSTRD)) in bytes. A value
	 * of 0h indicates a stride of 4 bytes, where the doorbell properties are packed without
	 * reserved space between each property. Refer to section 8.2.2.
	 *
	 * For message-based controllers, this field shall be cleared to a fixed value of 0h.
	 */
	QWORD DSTR:4;
	/**
	 * NVM Subsystem Reset Supported (NSSRS) - RO: This bit indicates whether the
	 * controller supports the NVM Subsystem Reset feature defined in section 3.7.1. This
	 * bit is set to '1' if the controller supports the NVM Subsystem Reset feature. This bit
	 * is cleared to ‘0' if the controller does not support the NVM Subsystem Reset feature.
	 * For Discovery controllers, this field shall be cleared to 0h.
	 */
	QWORD NSRS:1;
	/**
	 * Command Sets Supported (CSS) - RO: This field indicates the I/O Command Set(s)
	 * that the controller supports.
	 * For Discovery controllers, this field should have only the NCSS bit set to ‘1’.
	 * - 7
	 * No I/O Command Set Support (NOIOCSS): This bit indicates whether
	 * no I/O Command Set is supported (i.e., only the Admin Command Set
	 * is supported).
	 * This bit shall be set to ‘1’ if no I/O Command Set is supported.
	 * - 6
	 * I/O Command Set Support (IOCSS): This bit indicates whether the
	 * controller supports one or more I/O Command Sets and supports the
	 * Identify I/O Command Set data structure (refer to section 5.2.13.2.19).
	 * Controllers that support I/O Command Sets other than the NVM
	 * Command Set shall set this bit to ‘1’. Controllers that only support the
	 * NVM Command Set may set this bit to ‘1’.
	 * - 5:1 Reserved
	 * - 0
	 * NVM Command Set Support (NCSS): This bit indicates whether the
	 * controller supports the NVM Command Set or a Discovery controller.
	 */
	QWORD CSSU:8;
	/**
	 * Boot Partition Support (BPS) - RO: This bit indicates whether the controller supports
	 * Boot Partitions. If this bit is set to '1‘, then the controller supports Boot Partitions. If
	 * this bit is cleared to ‘0‘, then the controller does not support Boot Partitions. Refer
	 * to section 8.1.3.
	 */
	QWORD BPSU:1;
	/**
	 * Controller Power Scope (CPS) - RO: This field indicates scope of controlling the main
	 * power for this controller.
	 * - 00b Not Reported
	 * - 01b Controller scope
	 * - 10b Domain scope (i.e., the NVM subsystem supports multiple domains
	 * (refer to section 3.2.5).
	 * - 11b NVM subsystem scope (i.e., the NVM subsystem does not support
	 * multiple domains).
	 *
	 * If the NSSS bit is set to ‘1’, then this field shall not be cleared to 00b.
	 */
	QWORD CPWS:2;
	/**
	 * Memory Page Size Minimum (MPSMIN) - RO: This field indicates the minimum host
	 * memory page size that the controller supports. The minimum memory page size is
	 * (2 ^ (12 + MPSMIN)). The host shall not configure a memory page size in CC.MPS
	 * that is smaller than this value.
	 *
	 * For Discovery controllers this shall be cleared to 0h.
	 */
	QWORD MIPS:4;
	/**
	 * Memory Page Size Maximum (MPSMAX) - RO: This field indicates the maximum host
	 * memory page size that the controller supports. The maximum memory page size
	 * is (2 ^ (12 + MPSMAX)). The host shall not configure a memory page size in
	 * CC.MPS that is larger than this value.
	 *
	 * For Discovery controllers this field shall be cleared to 0h.
	 */
	QWORD MXPS:4;
	/**
	 * Persistent Memory Region Supported (PMRS) - RO: This bit indicates whether the
	 * Persistent Memory Region is supported. This bit is set to ‘1’ if the Persistent
	 * Memory Region is supported. This bit is cleared to ‘0’ if the Persistent Memory
	 * Region is not supported.
	 */
	QWORD PMRS:1;
	/**
	 * Controller Memory Buffer Supported (CMBS) - RO: If this bit is set to ‘1’, then this bit
	 * indicates that the controller supports the Controller Memory Buffer, and that
	 * addresses supplied by the host are permitted to reference the Controller Memory
	 * Buffer only if the host has enabled the Controller Memory Buffer’s controller
	 * memory space.
	 *
	 * If the controller supports the Controller Memory Buffer, then this bit shall be set to
	 * ‘1’.
	 */
	QWORD CMBS:1;
	/**
	 * NVM Subsystem Shutdown Supported (NSSS) - RO: This bit indicates whether the
	 * controller supports the NVM Subsystem Shutdown feature.
	 *
	 * If this bit is set to ‘1’, then the controller supports the NVM Subsystem Shutdown
	 * feature. If the NSSES bit is set to ‘1’, then this bit shall be set to ‘1’.
	 *
	 * If this bit is cleared to ‘0’, then the controller does not support the NVM Subsystem
	 * Shutdown feature. If the NSSRS bit is cleared to ‘0’, then this bit shall be cleared
	 * to ‘0’.
	 *
	 * Refer to section 3.6.3 for a description of the NVM Subsystem Shutdown feature
	 * and the behavioral enhancements associated with the NSSES bit being set to ‘1’.
	 */
	QWORD NSSS:1;
	/**
	 * Controller Ready Modes Supported (CRMS) - RO: This field indicates the ready
	 * capabilities of the controller. Refer to sections 3.5.3 and 3.5.4 for more detail.
	 * - 1
	 * Controller Ready Independent of Media Support (CRIMS): If this bit
	 * is set to ‘1’, then the controller supports the Controller Ready
	 * Independent of Media mode.
	 * If this bit is cleared to ‘0’, then the controller does not support Controller
	 * Ready Independent of Media mode.
	 * - 0
	 * Controller Ready With Media Support (CRWMS): If this bit is set to
	 * ‘1’, then the controller supports the Controller Ready With Media mode.
	 * If this bit is cleared to ‘0’, then the controller does not support Controller
	 * Ready With Media mode.
	 * This bit shall be set to ‘1’ on controllers compliant with NVM Express
	 * Base Specification, Revision 2.0 and later.
	 */
	QWORD CRMS:2;
	/**
	 * NVM Subsystem Shutdown Enhancements Supported (NSSES) - RO: This bit
	 * indicates whether the controller supports enhancements to the NVM Subsystem
	 * Shutdown feature.
	 *
	 * If the controller supports the enhancements to the NVM Subsystem Shutdown
	 * feature as defined in section 3.6.3, then this bit shall be set to ‘1’ and the NSSS bit
	 * shall be set to ‘1’. If a controller compliant with a revision of the NVM Express Base
	 * Specification later than revision 2.0 sets the NSSS bit to ‘1’, then that controller
	 * shall set this bit to ‘1’.
	 *
	 * If this bit is cleared to ‘0’, then the controller does not support the enhancements to
	 * the NVM Subsystem Shutdown feature as defined in section 3.6.3.
	 *
	 * If the NSSRS bit is cleared to ‘0’ or the NSSS bit is cleared to ‘0’, then this bit shall
	 * be cleared to ‘0’.
	 */
	QWORD SSES:1;
	QWORD RSV0:2;
} NVM_EXPRESS_CAPABILITY;
typedef struct _NVM_EXPRESS_CONTROLLER_MEMORY_BUFFER_LOCATION
{
	/**
	 * Base Indicator Register (BIR): Indicates the Base Address Register (BAR) that
	 * contains the Controller Memory Buffer. For a 64-bit BAR, the BAR for the least significant
	 * 32-bits of the address is specified. Values 000b, 010b, 011b, 100b, and 101b are valid.
	 * The address specified by the BAR shall be 4 KiB aligned.
	 */
	DWORD BIDR:3;
	/**
	 * CMB Queue Mixed Memory Support (CQMMS): If this bit is set to ‘1’, then for a
	 * particular queue placed in the Controller Memory Buffer, the restriction that all memory
	 * associated with that queue shall reside in the Controller Memory Buffer is not enforced
	 * (refer to section 8.2.1). If this bit is cleared to ‘0’, then that requirement is enforced.
	 */
	DWORD QMMS:1;
	/**
	 * CMB Queue Physically Discontiguous Support (CQPDS): If this bit is set to ‘1’, then
	 * the restriction that for all queues in the Controller Memory Buffer, the queue shall be
	 * physically contiguous, is not enforced (refer to section 8.2.1). If this bit is cleared to ‘0’,
	 * then that restriction is enforced.
	 */
	DWORD QPDS:1;
	/**
	 * CMB Data Pointer Mixed Locations Support (CDPMLS): If this bit is set to ‘1’, then
	 * the restriction that for a particular PRP List or SGL associated with a single command,
	 * all memory that contains that particular PRP List or SGL shall reside in either the
	 * Controller Memory Buffer or outside the Controller Memory Buffer, is not enforced (refer
	 * to section 8.2.1). If this bit is cleared to ‘0’, then that restriction is enforced.
	 */
	DWORD DPML:1;
	/**
	 * CMB Data Pointer and Command Independent Locations Support (CDPCILS): If this
	 * bit is set to ‘1’, then the restriction that the PRP Lists and SGLs shall not be located in
	 * the Controller Memory Buffer if the command that they are associated with is not located
	 * in the Controller Memory Buffer is not enforced (refer to section 8.2.1). If this bit is cleared
	 * to ‘0’, then that restriction is enforced.
	 */
	DWORD DPCI:1;
	/**
	 * CMB Data Metadata Mixed Memory Support (CDMMMS): If this bit is set to ‘1’, then
	 * the restriction on data and metadata use of Controller Memory Buffer by a command as
	 * defined in section 8.2.1 is not enforced. If this bit is cleared to ‘0’, then the restriction on
	 * data and metadata use of Controller Memory Buffer by a command as defined in section
	 * 8.2.1 is enforced.
	 */
	DWORD DMMM:1;
	/**
	 * CMB Queue Dword Alignment (CQDA): If this bit is set to ‘1’, CDW11.PC is set to ‘1’;
	 * and the address pointer specifies Controller Memory Buffer, then the address pointer in
	 * a Create I/O Submission Queue command (refer to Figure 506) or a Create I/O
	 * Completion Queue command (refer to Figure 502) shall be Dword aligned.
	 *
	 * If this bit is cleared to ‘0’, then the I/O Submission Queues and I/O Completion Queues
	 * contained in the Controller Memory Buffer are aligned as defined by the PRP1 field of a
	 * Create I/O Submission Queue command (refer to Figure 506) or a Create I/O Completion
	 * Queue command (refer to Figure 502).
	 */
	DWORD CQDA:1;
	DWORD RV06:3;
	// Offset (OFST): Indicates the offset of the Controller Memory Buffer in multiples of the Size Unit specified in CMBSZ.
	DWORD OFST:20;
} NVM_EXPRESS_CONTROLLER_MEMORY_BUFFER_LOCATION;
typedef struct _NVM_EXPRESS_CONTROLLER_SPACE
{
	NVM_EXPRESS_CAPABILITY CAPA;

	/**
	 * - 31:16 Major Version (MJR): An integer value indicating the major version number of this specification which is
	 * supported by the controller.
	 * - 15:08 Minor Version (MNR): An integer value indicating the minor version number of this specification which is
	 * supported by the controller.
	 * - 07:00
	 * Tertiary Version (TER): An integer value indicating the tertiary version number of this specification which
	 * is supported by the controller. If this field is cleared to 0h, then this specification does not have a tertiary
	 * version number.
	 */
	DWORD VERS;

	/**
	 * Interrupt Vector Mask Set (IVMS): This field is bit significant. If a ‘1’ is written to a
	 * bit, then the corresponding interrupt vector is masked from generating an interrupt
	 * or reporting a pending interrupt in the MSI Capability Structure. Writing a ‘0’ to a bit
	 * has no effect. When read, this field returns the current interrupt mask value within
	 * the controller (not the value of this property). If a bit has a value of a ‘1’, then the
	 * corresponding interrupt vector is masked. If a bit has a value of ‘0’, then the
	 * corresponding interrupt vector is not masked.
	 */
	DWORD IVMS;
	/**
	 * Interrupt Vector Mask Clear (IVMC): This field is bit significant. If a ‘1’ is written to
	 * a bit, then the corresponding interrupt vector is unmasked. Writing a ‘0’ to a bit has
	 * no effect. When read, this field returns the current interrupt mask value within the
	 * controller (not the value of this property). If a bit has a value of a ‘1’, then the
	 * corresponding interrupt vector is masked. If a bit has a value of ‘0’, then the
	 * corresponding interrupt vector is not masked.
	 */
	DWORD IVMC;

	/**
	 * Enable (EN): While set to ‘1’, then the controller shall process commands.
	 *
	 * While cleared to ‘0’, then the controller shall not process commands nor post
	 * completion queue entries to Completion Queues. If the host writes this property
	 * to clear this bit from ‘1’ to ‘0’, the controller is reset (i.e., a Controller Reset, refer
	 * to section 3.7.2.1). That Controller Reset results in a Controller Level Reset
	 * (refer to section 3.7.2) that deletes all I/O Submission Queues and I/O
	 * Completion Queues, resets the Admin Submission Queue and the Admin
	 * Completion Queue, and brings the hardware to an idle state. That Controller
	 * Level Reset does not affect transport specific state (e.g., PCI Express registers
	 * including MMIO MSI-X registers), nor the Admin Queue properties (AQA, ASQ,
	 * or ACQ). Refer to section 3.7.2 for the effects of that Controller Level Reset on
	 * all controller properties. Internal controller state (e.g., Feature values defined in
	 * section 5.2.26 that are not persistent across power states) are reset to their
	 * default values. The controller shall ensure that there is no impact (e.g., data
	 * loss) caused by that Controller Level Reset to the results of commands that
	 * have had corresponding completion queue entries posted to an I/O Completion
	 * Queue prior to that Controller Level Reset.
	 *
	 * When this bit is cleared to ‘0’, the CSTS.RDY bit is cleared to ‘0’ by the controller
	 * once the controller is ready to be enabled. When this bit is set to ‘1’, the
	 * controller sets the CSTS.RDY bit to ‘1’ when the controller is ready to process
	 * commands. The CSTS.RDY bit may be set to ‘1’ before namespace(s) are
	 * ready to be accessed.
	 *
	 * Setting this bit from a ‘0’ to a ‘1’ when the CSTS.RDY bit is a ‘1’ or clearing this
	 * bit from a '1' to a '0' when the CSTS.RDY bit is cleared to '0' has undefined
	 * results. The Admin Queue properties (AQA, ASQ, and ACQ) are only allowed
	 * to be modified when this bit is cleared to ‘0’.
	 *
	 * If an NVM Subsystem Shutdown is reported as in progress or is reported as
	 * completed (i.e., the CSTS.ST bit is set to ‘1’, and the CSTS.SHST field is set to
	 * 01b or 10b), then:
	 * - setting this bit from ‘0’ to ‘1’ modifies the field value but has no effect
	 * (e.g., the controller does not respond by setting the CSTS.RDY bit to
	 * ‘1’); and
	 * - clearing this bit from ‘1’ to ‘0’ resets the controller as defined by this
	 * field.
	 *
	 * Refer to section 3.6.3 for details on NVM Subsystem Shutdown functionality.
	 */
	DWORD ENAB:1;
	DWORD RSV2:3;
	/**
	 * I/O Command Set Selected (CSS): This field specifies the I/O Command Set
	 * or Sets that are selected. This field shall only be changed when the controller is
	 * disabled (i.e., CC.EN is cleared to ‘0’). The I/O Command Set or Sets that are
	 * selected shall be used for all I/O Submission Queues.
	 *
	 * - 000b CAP.CSS.NCSS
	 *   - 1b NVM Command Set
	 *   - 0b Reserved
	 *   - 001b to 101b Reserved
	 *
	 * - 110b CAP.CSS.IOCSS
	 *   - 1b
	 *   All Supported I/O Command Sets
	 *   The I/O Command Sets that are
	 *   supported are reported in the
	 *   Identify I/O Command Set data
	 *   structure (refer to section
	 *   5.2.13.2.19).
	 *   - 0b Reserved
	 *
	 * - 111b CAP.CSS.NOIOCSS
	 *   - 1b
	 *   Admin Command Set only
	 *   I/O Command Set and I/O
	 *   Command Set specific Admin
	 *   commands are not supported.
	 *   Any I/O Command Set
	 *   specific Admin command
	 *   submitted on the Admin
	 *   Submission Queue is aborted
	 *   with a status code of Invalid
	 *   Command Opcode.
	 *   - 0b Reserved
	 *
	 * For Discovery controllers, this property shall be cleared to 000b.
	 */
	DWORD ICSS:3;
	/**
	 * Memory Page Size (MPS): This field indicates the host memory page size. The
	 * memory page size is (2 ^ (12 + MPS)). Thus, the minimum host memory page
	 * size is 4 KiB and the maximum host memory page size is 128 MiB. The value
	 * set by the host shall be a supported value as indicated by the CAP.MPSMAX
	 * and CAP.MPSMIN fields. If the host sets this field to a value that is not
	 * supported (i.e., a value that is greater than CAP.MPSMAX or a value that is less
	 * than CAP.MPSMIN), then controller behavior is undefined. This field also
	 * describes the value used for PRP entry size. This field shall only be modified by
	 * the host while CC.EN is cleared to ‘0’. If this field is modified by the host while
	 * CC.EN is set to ‘1’, then controller behavior is undefined.
	 *
	 * For Discovery controllers this property shall be cleared to 0h.
	 */
	DWORD MPGS:4;
	/**
	 * Arbitration Mechanism Selected (AMS): This field selects the arbitration
	 * mechanism to be used. This value shall only be changed when CC.EN is
	 * cleared to ‘0’. The host shall only set this field to supported arbitration
	 * mechanisms indicated in CAP.AMS. If this field is set to an unsupported value,
	 * then controller behavior is undefined.
	 *
	 * For Discovery controllers, this value shall be cleared to 0h.

	 * - 000b Round Robin
	 * - 001b Weighted Round Robin with Urgent Priority Class
	 * - 010b to 110b Reserved
	 * - 111b Vendor Specific
	 */
	DWORD AMSL:3;
	/**
	 * Shutdown Notification (SHN) - RW: This field is used to initiate a controller
	 * shutdown when a power down condition is expected. For a normal controller
	 * shutdown, it is expected that the controller is given time to process the controller
	 * shutdown before power is removed. For an abrupt shutdown, the host may or
	 * may not wait for the controller shutdown to complete before power is removed.
	 *
	 * The controller shutdown notification values are defined as:
	 * - 00b No notification and no effect
	 * - 01b Normal shutdown notification
	 * - 10b Abrupt shutdown notification
	 * - 11b Reserved
	 *
	 * This field should be written by the host prior to any power down condition and
	 * prior to any change of the PCI power management state. It is recommended
	 * that this field also be written prior to a warm reset (refer to the PCI Express
	 * Base Specification). To determine when the controller shutdown processing is
	 * complete, refer to the definition of the CSTS.ST bit and the definition of the
	 * CSTS.SHST field. Refer to section 3.6 for additional shutdown processing
	 * details.
	 *
	 * Other fields in this property (including the EN bit) may be modified as part of
	 * updating this field to 01b or 10b to initiate a controller shutdown. If the host write
	 * to this property that initiates a shutdown also clears the EN bit from ‘1’ to ‘0’,
	 * then both a Controller Reset and a controller shutdown occur.
	 *
	 * If an NVM Subsystem Shutdown is reported as in progress or is reported as
	 * complete (i.e., CSTS.ST is set to ‘1’, and CSTS.SHST is set to either 01b or
	 * 10b), then writes to this field modify the field value but have no effect. Refer to
	 * section 3.6.3 for details.
	 */
	DWORD STDN:2;
	/**
	 * I/O Submission Queue Entry Size (IOSQES): This field defines the I/O
	 * submission queue entry size that is used for the selected I/O Command Set(s).
	 * The required and maximum values for this field are specified in the SQES field
	 * in the Identify Controller data structure in Figure 328 for each I/O Command
	 * Set. The value is in bytes and is specified as a power of two (2^n).
	 *
	 * If any I/O Submission Queues exist, then write operations that change the value
	 * in this field produce undefined results.
	 *
	 * If the controller does not support I/O queues, then this field shall be read-only
	 * with a value of 0h.
	 *
	 * For Discovery controllers, this field is reserved.
	 */
	DWORD ISQE:4;
	/**
	 * I/O Completion Queue Entry Size (IOCQES): This field defines the I/O
	 * completion queue entry size that is used for the selected I/O Command Set(s).
	 * The required and maximum values for this field are specified in the CQES field
	 * in the Identify Controller data structure in Figure 328 for each I/O Command
	 * Set. The value is in bytes and is specified as a power of two (2^n).
	 *
	 * If any I/O Completion Queues exist, then write operations that change the value
	 * in this field produce undefined results.
	 *
	 * If the controller does not support I/O queues, then this field shall be read-only
	 * with a value of 0h.
	 *
	 * For Discovery controllers, this field is reserved.
	 */
	DWORD ICQE:4;
	/**
	 * Controller Ready Independent of Media Enable (CRIME): This bit controls
	 * the controller ready mode. The controller ready mode is determined by the state
	 * of this bit at the time the controller is enabled by transitioning the CC.EN bit from
	 * ‘0’ to ‘1’.
	 *
	 * If the CAP.CRMS field is set to 11b, then this bit is RW. If the CAP.CRMS field
	 * is not set to 11b, then this bit is RO and shall be cleared to ‘0’. Refer to sections
	 * 3.5.3 and 3.5.4 for more detail.
	 *
	 * Changing the value of this field may cause a change in the time reported in the
	 * CAP.TO field. Refer to the definition of CAP.TO for more details.
	 *
	 * - 0b
	 * Controller Ready With Media Mode: Enabling the controller (i.e.,
	 * CC.EN transitions from ‘0’ to ‘1’) when this bit is cleared to ‘0’
	 * enables Controller Ready With Media mode.
	 * - 1b
	 * Controller Ready Independent Of Media Mode: Enabling the
	 * controller when this bit is set to ‘1’ enables Controller Ready
	 * Independent of Media mode.
	 */
	DWORD CRIM:1;
	DWORD RSV3:7;

	DWORD RSV5;

	/**
	 * Ready (RDY): This bit is set to ‘1’ when the controller is ready to process submission
	 * queue entries after the CC.EN bit is set to ‘1’. This bit shall be cleared to ‘0’ when the
	 * CC.EN bit is cleared to ‘0’ once the controller is ready to be re-enabled. Commands
	 * should not be submitted to the controller until this bit is set to ‘1’ after the CC.EN bit is
	 * set to ‘1’. Failure to follow this recommendation produces undefined results. Refer to
	 * the definition of the CAP.TO field, section 3.5.3, and section 3.5.4 for timing
	 * information related to this field.
	 *
	 * If an NVM Subsystem Shutdown that affects this controller is reported as in progress
	 * or is reported as complete (i.e., the CSTS.ST bit is set to ‘1’ and the CSTS.SHST field
	 * is set to 01b or is set to 10b), then an NVM Subsystem Reset is required before this
	 * bit is allowed to be set to ‘1’ from ‘0’. Refer to section 3.6.3.
	 *
	 * If a controller shutdown is reported as in progress or is reported as complete (i.e., the
	 * CSTS.ST bit is cleared to ‘0’ and the CSTS.SHST field is set to 01b or is set to 10b),
	 * then before this bit is allowed to be set to ‘1’ from ‘0’, controller shutdown processing
	 * shall stop (e.g., complete or be terminated) and the CSTS.SHST field shall be cleared
	 * to 00b.
	 */
	DWORD RADY:1;
	/**
	 * Controller Fatal Status (CFS): This bit is set to ’1’ when a fatal controller error
	 * occurred that could not be communicated in the appropriate Completion Queue. This
	 * bit is cleared to ‘0’ when a fatal controller error has not occurred. Refer to section 9.5.
	 *
	 * The reset value of this bit is set to '1' when a fatal controller error is detected during
	 * controller initialization.
	 */
	DWORD CFST:1;
	/**
	 * Shutdown Status (SHST) - RO: This field indicates the status of shutdown processing that
	 * is initiated by the host setting the CC.SHN field, the host setting the NSSD.NSSC field,
	 * or a Management Endpoint processing an NVMe-MI Shutdown command (refer to the
	 * NVM Express Management Interface Specification). Shutdown processing is able to
	 * occur on this controller as a consequence of a host setting the NSSD.NSSC field on
	 * another controller to initiate an NVM Subsystem Shutdown that affects this controller.
	 *
	 * The shutdown status values are defined as:
	 * - 00b Normal operation (no shutdown has been requested)
	 * - 01b Shutdown processing in progress
	 * - 10b Shutdown processing complete
	 * - 11b Reserved
	 *
	 * If this field is set to 01b (i.e., shutdown processing in progress), then:
	 * - an NVM Subsystem Reset aborts both a controller shutdown and an NVM
	 * Subsystem Shutdown; and
	 * - any other type of Controller Level Reset (CLR):
	 *   - may or may not abort a controller shutdown; and
	 *   - shall not abort an NVM Subsystem Shutdown.
	 *
	 * If this field is cleared to 00b (i.e., normal operation) when a CLR is initiated, then that
	 * CLR shall not change the value of this field.
	 *
	 * If this field is set to 01b when a CLR is initiated, and shutdown processing is not
	 * aborted by that CLR, then that CLR shall not change the value of this field.
	 *
	 * If this field is set to 01b when a CLR is initiated and shutdown processing is aborted
	 * by that CLR, then that CLR shall clear this field to 00b.
	 *
	 * If this field is set to 10b (i.e., shutdown processing complete) when a CLR is initiated
	 * by NVM Subsystem Reset, then that CLR shall clear this field to 00b.
	 *
	 * If this field is set to 10b when a CLR is initiated by a method other than NVM
	 * Subsystem Reset and:
	 * - the CSTS.ST bit is set to ‘1’, then that CLR shall not change the value of this
	 * field; and
	 * - the CSTS.ST bit is cleared to ‘0’, then that CLR shall clear this field to 00b.
	 *
	 * If the CSTS.ST bit is cleared to ‘0’ and this field is set to 10b (i.e., controller shutdown
	 * processing is reported as complete), then to start executing commands on the
	 * controller:
	 * - if the CC.EN bit is set to ‘1’, then a CLR initiated by any method (e.g., a
	 * Controller Reset) followed by enabling the controller (i.e., host sets the
	 * CC.EN bit from ‘0’ to ‘1’) is required (refer to section 3.6.1). If a host submits
	 * commands to the controller without a prior CLR, then the behavior is
	 * undefined; and
	 * - if the CC.EN bit is cleared to ‘0’, then:
	 *   - a CLR followed by enabling the controller is required (refer to
	 *   sections 3.6.1 and 3.6.2); or
	 *   - the CC.EN bit is required to be set to ‘1’ and the CC.SHN field is
	 *   required to be cleared to 00b with the same write to the CC property
	 *   (refer to sections 3.6.1 and 3.6.2).
	 *
	 * If the CSTS.ST bit is set to ‘1’ and this field is set to 10b (i.e., NVM Subsystem
	 * Shutdown processing is reported as complete), then an NVM Subsystem Reset
	 * followed by enabling the controller is required to start executing commands (refer to
	 * section 3.6.3). If a host submits commands to the controller without a prior NVM
	 * Subsystem Reset, then the behavior is undefined.
	 */
	DWORD STDS:2;
	/**
	 * NVM Subsystem Reset Occurred (NSSRO) - RW1C: The initial value of this bit is set to '1' if
	 * the last occurrence of an NVM Subsystem Reset (refer to section 3.7.1) occurred
	 * while power was applied to the domain. The initial value of this bit is cleared to '0'
	 * following an NVM Subsystem Reset due to application of power to the domain. This
	 * bit is only valid if the controller supports the NVM Subsystem Reset feature defined in
	 * section 3.7.1 as indicated by CAP.NSSRS set to ‘1’.
	 * The reset value of this bit is cleared to '0' if an NVM Subsystem Reset causes
	 * activation of a new firmware image in the domain.
	 */
	DWORD NSRO:1;
	/**
	 * Processing Paused (PP) - RO: This bit indicates whether the controller is processing
	 * commands. If this bit is cleared to ‘0’, then the controller is processing commands
	 * normally. If this bit is set to ‘1’, then the controller has temporarily stopped processing
	 * commands in order to handle an event (e.g., firmware activation). This bit is only valid
	 * when CC.EN is set to ‘1’ and CSTS.RDY is set to ‘1’.
	 */
	DWORD PRCP:1;
	/**
	 * Shutdown Type (ST) - RO: If CSTS.SHST is set to a non-zero value, then this bit indicates
	 * the type of shutdown reported by CSTS.SHST.
	 *
	 * If this bit is set to ‘1’, then CSTS.SHST is reporting the state of an NVM Subsystem
	 * Shutdown and this bit remains set to ‘1’ until an NVM Subsystem Reset occurs.
	 * If this bit is cleared to ‘0’, then CSTS.SHST is reporting the state of a controller
	 * shutdown.
	 *
	 * An NVM Subsystem Reset shall clear this bit to ‘0’. All other Controller Level Resets
	 * shall not change the value of this bit.
	 * If CSTS.SHST is cleared to 00b, then this bit should be ignored by the host.
	 */
	DWORD STDT:1;
	DWORD RSV4:25;

	/**
	 * NVM Subsystem Reset Control (NSSRC) - RW: A write of the value 4E564D65h ("NVMe")
	 * to this field initiates an NVM Subsystem Reset. A write of any other value has no
	 * functional effect on the operation of the NVM subsystem. This field shall return the
	 * value 0h when read.
	 */
	DWORD NSRC;

	/**
	 * Admin Submission Queue Size (ASQS) - RW: Defines the size of the Admin Submission
	 * Queue in entries. Refer to section 3.3.3.1. Enabling a controller while this field is cleared
	 * to 0h produces undefined results. The minimum size of the Admin Submission Queue is
	 * two entries. The maximum size of the Admin Submission Queue is 4,096 entries. This is
	 * a 0’s based value.
	 */
	DWORD ASQS:16;
	/**
	 * Admin Completion Queue Size (ACQS) - RW: Defines the size of the Admin Completion
	 * Queue in entries. Refer to section 3.3.3.1. Enabling a controller while this field is cleared
	 * to 0h produces undefined results. The minimum size of the Admin Completion Queue is
	 * two entries. The maximum size of the Admin Completion Queue is 4,096 entries. This is
	 * a 0’s based value.
	 */
	DWORD ACQS:16;

	/**
	 * Admin Submission Queue Base (ASQB) - RW: This field specifies the 52 most significant
	 * bits of the 64-bit physical address for the Admin Submission Queue. This address shall
	 * be memory page aligned (based on the value in CC.MPS). All Admin commands,
	 * including creation of I/O Submission Queues and I/O Completions Queues shall be
	 * submitted to this queue. For the definition of Submission Queues, refer to section 4.1.
	 */
	QWORD ASQA;

	/**
	 * Admin Completion Queue Base (ACQB) - RW: This field specifies the 52 most significant
	 * bits of the 64-bit physical address for the Admin Completion Queue. This address shall
	 * be memory page aligned (based on the value in CC.MPS). All completion queue entries
	 * for the commands submitted to the Admin Submission Queue shall be posted to this
	 * Completion Queue. This queue is always associated with interrupt vector 0. For the
	 * definition of Completion Queues, refer to section 4.1.
	 */
	QWORD ACQA;

	DWORD CMBL;
	DWORD CMBS;
} NVM_EXPRESS_CONTROLLER_SPACE;
typedef struct _NVM_EXPRESS_CONTROLLER NVM_EXPRESS_CONTROLLER;
struct _NVM_EXPRESS_CONTROLLER
{
	PCI_EXPRESS_DEVICE            PCIE;
	NVM_EXPRESS_CONTROLLER       *NEXT;
	NVM_EXPRESS_CONTROLLER_SPACE *NVME;
};

void nvme_controller_setup(PCI_EXPRESS_DEVICE *pcie);