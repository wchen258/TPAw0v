#include "trace.h"

extern uint32_t read_data(uint8_t*, uint32_t, uint8_t);
extern uint32_t advance_pointer(uint32_t);
extern uint8_t data_available();

static address_reg_t address_regs[3];

static uint8_t trace_state = 0;

static void init_address_regs(void) {
    uint8_t i;
    for (i = 0; i < 3; ++i) {
        address_regs[i].address = 0x0;
        address_regs[i].is = 0x0;
    }
}

static void update_address_regs(uint64_t address, uint8_t is) {
    address_regs[2] = address_regs[1];
    address_regs[1] = address_regs[0];
    address_regs[0].address = address;
    address_regs[0].is = is;
}

static void handle_atom(uint8_t status) {
    if (status) {
        report("E atom");
    } else {
        report("N atom");
    }
    report_atom(status);

}

static void handle_address(uint64_t address, uint8_t is) {
    report("address: 0x%lx, is: %d", address, is);
    report_addres(address, is);
    update_address_regs(address, is);
}

static void start_trace(void) {
    if (trace_state == 0 || trace_state == 3) {
        trace_state = 1;
        report("Starting/Resuming trace");
    }
}

static void end_trace(void) {
    if (trace_state == 1) {
        trace_state = 2;
        report("Trace ended");
    }
}

static void pause_trace(void) {
    if (trace_state == 1) {
        trace_state = 3;
        report("Trace paused until SYNC");
    }
}

void trace_loop(void) {
    uint8_t header;
    uint32_t packet_counter = 0;

    init_address_regs();

    while(data_available()) {
        read_data(&header, 1, 1);

        if (trace_state == 2 || ((trace_state == 0 || trace_state == 3) && header != 0x0)) {
            report("byte 0x%x outside of trace scope", header);
            continue;
        }

        report("Pkt: %d Header: %#04x", packet_counter, header);
        packet_counter++;

        switch (header)
        {
        case Async:
            handle_async();
            break;
        case Resync:
            handle_resync();
            break;
        case TraceInfo:
            handle_traceinfo();
            break;
        case Context0:
        case Context1:
            handle_context(header);
            break;
        case LongAddress0:
        case LongAddress1:
        case LongAddress2:
        case LongAddress3:
            handle_longaddress(header);
            break;
        case ShortAddr0:
        case ShortAddr1:
            handle_shortaddress(header);
            break;
        case ExactMatch0:
        case ExactMatch1:
        case ExactMatch2:
            handle_exactmatch(header);
            break;
        case AddrWithContext0:
        case AddrWithContext1:
        case AddrWithContext2:
        case AddrWithContext3:
            handle_addrwithcontext(header);
            break;
        case TimeStamp0:
        case TimeStamp1:
            handle_timestamp(header);
            break;
        case Atom10:
        case Atom11:
            handle_atom1(header);
            break;
        case Atom20:
        case Atom21:
        case Atom22:
        case Atom23:
            handle_atom2(header);
            break;
        case Atom40:
        case Atom41:
        case Atom42:
        case Atom43:
            handle_atom4(header);
            break;
        case Atom50:
        case Atom51:
        case Atom52:
        case Atom53:
            handle_atom5(header);
            break;
        case Exce:
            handle_exception();
            break;
        case ExceReturn:
            handle_exceptionreturn();
            break;
        case FunctionReturn:
            handle_functionreturn();
            break;
        case TraceOn:
            handle_traceon();
            break;
        case CCF10:
        case CCF11:
            handle_ccf1(header);
            break;
        case CCF20:
        case CCF21:
            handle_ccf2(header);
            break;
        default:
            if (header >= 0b11111000) {
                handle_atom3(header);
            } else if ((header >= 0b11000000 && header <= 0b11010100) 
                        || (header >= 0b11100000 && header <= 0b11110100)) {
                handle_atom6(header);
            } else if (header >= 0b01110001 && header <= 0b01111111) {
                handle_event(header);
            } else if (header >= 0b00010000 && header <= 0b00011111) {
                handle_ccf3(header);
            } else {
                report("Undefined header 0x%x", header);
            }
            break;
        }

        report("");
    }
}

void handle_async(void) {
    uint8_t payload[10]; 
    read_data(payload, 1, 1);

    if (payload[0] == 0x5){ 
        report("Async, OVERFLOW detected");
	}
    else if (payload[0] == 0x3) {
        report ("Async, Disarcrd.");
    } else if (payload[0] == 0x7) {
        report ("Async, Branch Future Flush.");
    } else if (payload[0] == 0) {
        read_data(payload, 10, 1);
        if (payload[0] == 0x0 && payload[1] == 0x0 && payload[2] == 0x0 && payload[3] == 0x0
                && payload[4] == 0x0 && payload[5] == 0x0 && payload[6] == 0x0 && payload[7] == 0x0
                && payload[8] == 0x0 && payload[9] == 0x80) {
            start_trace();
            report("Async, OK");
        } else {
            report("Async, other. Ending trace.");
            //end_trace();
            pause_trace();
        }
    }
}

void handle_resync(void) {
    report("Resync");
}

void handle_traceinfo(void) {
    uint8_t header;
    uint8_t payload[2];
    read_data(&header, 1, 1);

    switch (header)
    {
    case 0b00000001:
        read_data(payload, 1, 1);
        if (payload[0] == 0b00000000) { // TODO: figure this out
            report("INFO session: nothing is enabled. No continue byte.");
        } else {
            report("UNFINISHED handle_traceinfo(0b00000001) header");
        }
        break;
    case 0b00001001:
        read_data(payload, 2, 1);
        report("Cycle Count enable");
        report("CC: %d", payload[1]);
        break;
    case 0b00000000:
        report("The trace might have ended!");
        //end_trace();
        pause_trace();
        break;
    default:
        report("UNFINISHED handle_traceinfo header");
        break;
    }
}

void handle_context(uint8_t header) {
    uint8_t context_info, vmid; // On our system (Cortex-A53), vmid is only one byte
    uint32_t contextid;

    if ((header & 0x1) == 0) {
        report("Context packet, no payload");
    } else {
        report("Context packet with payload");
        read_data(&context_info, 1, 1);
        report("payload ctl: 0x%x", context_info);
        if (((context_info >> 6) & 1) == 1) {
            read_data(&vmid, 1, 1);
            report("vmid: %d", vmid);
        }
        if ((context_info >> 7) == 1) {
            read_data((uint8_t*) &contextid, 4, 1);
            report("contextid: %d", contextid);
        }
    }
}

void handle_longaddress(uint8_t header) {
    uint8_t payload[8];
    uint8_t is;
    uint64_t address = address_regs[0].address;

    switch (header)
    {
    case 0b10011010:
        report("Long Address 32 IS0 packet");
        read_data(payload, 4, 1);
        is = 0;
        address = address & ~((uint64_t) 0xffffffff);
        address = address | (((uint64_t) payload[0] & 0x7f) << 2)
                        | (((uint64_t) payload[1] & 0x7f) << 9)
                        | (((uint64_t) payload[2]) << 16)
                        | (((uint64_t) payload[3]) << 24);
        break;
    case 0b10011011:
        report("Long Address 32 IS1 packet");
        read_data(payload, 4, 1);
        is = 1;
        address = address & ~((uint64_t) 0xffffffff);
        address = address | (((uint64_t) payload[0] & 0x7f) << 1)
                        | (((uint64_t) payload[1]) << 8)
                        | (((uint64_t) payload[2]) << 16)
                        | (((uint64_t) payload[3]) << 24);
        break;
    case 0b10011101:
        report("Long Address 64 IS0 packet");
        read_data(payload, 8, 1);
        is = 0;
        address = 0;
        address = address | (((uint64_t) payload[0] & 0x7f) << 2)
                        | (((uint64_t) payload[1] & 0x7f) << 9)
                        | (((uint64_t) payload[2]) << 16)
                        | (((uint64_t) payload[3]) << 24)
                        | (((uint64_t) payload[4]) << 32)
                        | (((uint64_t) payload[5]) << 40)
                        | (((uint64_t) payload[6]) << 48)
                        | (((uint64_t) payload[7]) << 56);
        break;
    case 0b10011110:
        report("Long Address 64 IS1 packet");
        read_data(payload, 8, 1);
        is = 1;
        address = 0;
        address = address | (((uint64_t) payload[0] & 0x7f) << 1)
                        | (((uint64_t) payload[1]) << 8)
                        | (((uint64_t) payload[2]) << 16)
                        | (((uint64_t) payload[3]) << 24)
                        | (((uint64_t) payload[4]) << 32)
                        | (((uint64_t) payload[5]) << 40)
                        | (((uint64_t) payload[6]) << 48)
                        | (((uint64_t) payload[7]) << 56);
        break;
    default:
        report("UNDEFINED handle_longaddress header");
        break;
    }

    handle_address(address, is);
}

void handle_shortaddress(uint8_t header) {
    uint8_t payload;
    uint8_t is;
    uint64_t address = address_regs[0].address;

    switch (header)
    {
    case 0b10010101:
        report("Short Address IS0 packet");
        read_data(&payload, 1, 1);
        is = 0;
        address = address & ~((uint64_t) 0x1ff);
        address = address | (((uint64_t) payload & 0x7f) << 2);
        if ((payload >> 7) == 1) {
            read_data(&payload, 1, 1);
            address = address & ~0x1fe00;
            address = address | (((uint64_t) payload) << 9);
        }
        break;
    case 0b10010110:
        report("Short Address IS1 packet");
        read_data(&payload, 1, 1);
        is = 1;
        address = address & ~((uint64_t) 0xff);
        address = address | (((uint64_t) payload & 0x7f) << 1);
        if ((payload >> 7) == 1) {
            read_data(&payload, 1, 1);
            address = address & ~0xff00;
            address = address | (((uint64_t) payload) << 8);
        }
        break;
    default:
        report("UNDEFINED handle_shortaddress header");
        break;
    }

    handle_address(address, is);
}

void handle_exactmatch(uint8_t header) {
    uint8_t index = header & 0b11;
    report("Exact Match Address(%d)", index);
    handle_address(address_regs[index].address, address_regs[index].is);
}

void handle_addrwithcontext(uint8_t header) {
    report("Address with context");

    switch (header)
    {
    case 0b10000010:
        handle_longaddress(0b10011010);
        handle_context(0b10000001);
        break;
    case 0b10000011:
        handle_longaddress(0b10011011);
        handle_context(0b10000001);
        break;
    case 0b10000101:
        handle_longaddress(0b10011101);
        handle_context(0b10000001);
        break;
    case 0b10000110:
        handle_longaddress(0b10011110);
        handle_context(0b10000001);
        break;
    default:
        report("UNDEFINED handle_addrwithcontext header");
        break;
    }
}

void handle_timestamp(uint8_t header) {
    uint8_t i = 0, payload;
    uint64_t timestamp = 0;
    uint32_t count = 0;

    report("Timestamp packet");

    do {
        read_data(&payload, 1, 1);
        if (i != 8) {
            timestamp = timestamp | (((uint64_t) payload & 0x7f) << (7 * i));
        } else {
            timestamp = timestamp | (((uint64_t) payload) << (7 * i));
        }
    } while(((payload >> 7) == 1) && (++i < 9));

    report("timestamp: %d", timestamp);

    if ((header & 0x1) == 1) {
        i = 0;

        do {
            read_data(&payload, 1, 1);
            if (i != 2) {
                count = count | (((uint64_t) payload & 0x7f) << (7 * i));
            } else {
                count = count | (((uint64_t) payload & 0x3f) << (7 * i));
            }
        } while (((payload >> 7) == 1) && (++i < 3));

        report("count: %d", count);
    } else {
        report("no count info");
    }
}

void handle_atom1(uint8_t header) {
    report("Atom 1 packet");
    handle_atom(header & 0x1);
}

void handle_atom2(uint8_t header) {
    report("Atom 2 packet");
    handle_atom(header & 0x1);
    handle_atom((header >> 1) & 0x1);
}

void handle_atom3(uint8_t header) {
    report("Atom 3 packet");
    handle_atom(header & 0x1);
    handle_atom((header >> 1) & 0x1);
    handle_atom((header >> 2) & 0x1);
}

void handle_atom4(uint8_t header) {
    report("Atom 4 packet");
    switch (header & 0b11)
    {
    case 0b00:
        handle_atom(0);
        handle_atom(1);
        handle_atom(1);
        handle_atom(1);
        break;
    case 0b01:
        handle_atom(0);
        handle_atom(0);
        handle_atom(0);
        handle_atom(0);
        break;
    case 0b10:
        handle_atom(0);
        handle_atom(1);
        handle_atom(0);
        handle_atom(1);
        break;
    case 0b11:
        handle_atom(1);
        handle_atom(0);
        handle_atom(1);
        handle_atom(0);
        break;
    }
}

void handle_atom5(uint8_t header) {
    report("Atom 5 packet");
    switch (((header >> 3) & 0b100) | (header & 0b11))
    {
    case 0b101:
        handle_atom(0);
        handle_atom(1);
        handle_atom(1);
        handle_atom(1);
        handle_atom(1);
        break;
    case 0b001:
        handle_atom(0);
        handle_atom(0);
        handle_atom(0);
        handle_atom(0);
        handle_atom(0);
        break;
    case 0b010:
        handle_atom(0);
        handle_atom(1);
        handle_atom(0);
        handle_atom(1);
        handle_atom(0);
        break;
    case 0b011:
        handle_atom(1);
        handle_atom(0);
        handle_atom(1);
        handle_atom(0);
        handle_atom(1);
        break;
    default:
        report("UNDEFINED handle_atom5 case: 0x%x", header);
        break;
    }
}

void handle_atom6(uint8_t header) {
    uint8_t i;
    report("Atom 6 packet");

    for (i = 0; i <= ((header & 0b11111) + 2); ++i) {
        handle_atom(1);
    }

    handle_atom(!((header >> 5) & 0x1));
}

void handle_event(uint8_t header) {
    uint8_t field = header & 0xf;
    report("Events: %d%d%d%d", (field >> 3) & 0x1, (field >> 2) & 0x1, (field >> 1) & 0x1, field & 0x1);
}

void handle_exception(void) {
    uint8_t payload, efield, pfield;
    uint16_t type;

    report("Exception packet");
    read_data(&payload, 1, 1);

    efield = (((payload >> 5) & 0b10) | (payload & 0b1)) & 0b11;
    type = (payload >> 1) & 0b11111;

    if ((payload >> 7) == 1) {
        read_data(&payload, 1, 1);
        type = type | (((uint16_t) payload & 0b11111) << 5);
        pfield = (payload >> 5) & 0x1;

        report(pfield ? "serious fault pending" : "no serious fault pending");
    }

    report("type: 0x%x, e: %d", type, efield);

    switch (type)
    {
    case 0b00000:
        report("PE reset");
        break;
    case 0b00001:
        report("Debug halt");
        break;
    case 0b00010:
        report("Call");
        break;
    case 0b00011:
        report("Trap");
        break;
    case 0b00100:
        report("System error");
        break;
    case 0b00110:
        report("Inst debug");
        break;
    case 0b00111:
        report("Data debug");
        break;
    case 0b01010:
        report("Alignment");
        break;
    case 0b01011:
        report("Inst fault");
        break;
    case 0b01100:
        report("Data fault");
        break;
    case 0b01110:
        report("IRQ");
        break;
    case 0b01111:
        report("FIQ");
        break;
    default:
        report("reserved type");
        break;
    }
}

void handle_exceptionreturn(void) {
    report("Exception return packet");
}

void handle_functionreturn(void) {
    report("Function return packet");
}

void handle_traceon(void) {
    report("Trace on packet");
}

void handle_ccf1(uint8_t header) {
    uint8_t payload, i = 0;
    uint32_t count = 0;

    report("CCF1 packet");

    if ((header & 0x1) == 0) {
        do {
            read_data(&payload, 1, 1);

            if (i != 2) {
                count = count | (((uint32_t) payload & 0x7f) << (7 * i));
            } else {
                count = count | (((uint32_t) payload & 0x3f) << (7 * i));
            }
        } while (((payload >> 7) == 1) && (++i < 3));

        report("cc: %d", count + CC_THRESHOLD);
    } else {
        report("CC unknown skip");
    }
}

void handle_ccf2(uint8_t header) {
    (void) header; // should be used, this is not complete functionality of ccf2
    uint8_t payload;
    report("CCF2 packet");
    read_data(&payload, 1, 1);
    report("cc: %d", (payload & 0b1111) + CC_THRESHOLD);
}

void handle_ccf3(uint8_t header) {
    report("CCF3 packet");
    report("cc: %d", (header & 0b11) + CC_THRESHOLD);
}
