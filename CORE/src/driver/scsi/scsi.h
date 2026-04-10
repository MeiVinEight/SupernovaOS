//
// Created by MeiVi on 2026/04/08.
//

#pragma once

#include <types.h>

WORD scsi_reverse2(WORD);
DWORD scsi_reverse4(DWORD);
QWORD scsi_reverse8(QWORD);
void scsi_reverse(BYTE *data, DWORD len);