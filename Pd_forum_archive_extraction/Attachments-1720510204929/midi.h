/*     
 **********************************************************************
 *     midi.h
 *     Copyright 1999, 2000 Creative Labs, Inc. 
 * 
 ********************************************************************** 
 * 
 *     Date                 Author          Summary of changes 
 *     ----                 ------          ------------------ 
 *     October 20, 1999     Bertrand Lee    base code release 
 * 
 ********************************************************************** 
 * 
 *     This program is free software; you can redistribute it and/or 
 *     modify it under the terms of the GNU General Public License as 
 *     published by the Free Software Foundation; either version 2 of 
 *     the License, or (at your option) any later version. 
 * 
 *     This program is distributed in the hope that it will be useful, 
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *     GNU General Public License for more details. 
 * 
 *     You should have received a copy of the GNU General Public 
 *     License along with this program; if not, write to the Free 
 *     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, 
 *     USA. 
 * 
 ********************************************************************** 
 */ 

#ifndef _MIDI_H
#define _MIDI_H

#define FMODE_MIDI_SHIFT 3
#define FMODE_MIDI_READ  (FMODE_READ << FMODE_MIDI_SHIFT)
#define FMODE_MIDI_WRITE (FMODE_WRITE << FMODE_MIDI_SHIFT)

#define MIDIIN_STATE_STARTED 0x00000001
#define MIDIIN_STATE_STOPPED 0x00000002

#define MIDIIN_BUFLEN 1024

struct emu10k1_mididevice
{
	struct emu10k1_card *card;
	u32 mistate;
	wait_queue_head_t oWait;
	wait_queue_head_t iWait;
	s8 iBuf[MIDIIN_BUFLEN];
	u16 ird, iwr, icnt;
	struct midi_hdr *pmiHdrList;
};

#endif /* _MIDI_H */
