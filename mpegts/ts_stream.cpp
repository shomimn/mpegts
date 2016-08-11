#include "ts_stream.h"

extern "C"
{
#include <io.h>
#include <fcntl.h>
}

ts_stream::ts_stream()
	: fd(-1)
	, pat(this)
	, any_packet_handler(nullptr)
	, start_handler(nullptr)
	, end_handler(nullptr)
{
}

ts_stream::~ts_stream()
{
	if (fd > -1)
		_close(fd);
}

int ts_stream::read(ts_packet& packet)
{
	int count = _read(fd, &packet.data[0], 1);
	while (packet.data[0] != 0x47 && count > 0)
		count = _read(fd, &packet.data[0], 1);

	count += _read(fd, &packet.data[1], 187);
	packet.init(count);

	return count;
}

void ts_stream::on_packet(int pid, packet_fn& f)
{
	packet_handlers.insert({ pid, f });
}

void ts_stream::on_packet(int pid, packet_fn&& f)
{
	packet_handlers.insert({ pid, std::move(f) });
}

void ts_stream::on_any_packet(packet_fn& f)
{
	any_packet_handler = f;
}

void ts_stream::on_any_packet(packet_fn&& f)
{
	any_packet_handler = std::move(f);
}

void ts_stream::on_start(nullary_fn& f)
{
	start_handler = f;
}

void ts_stream::on_start(nullary_fn&& f)
{
	start_handler = std::move(f);
}

void ts_stream::on_end(nullary_fn& f)
{
	end_handler = f;
}

void ts_stream::on_end(nullary_fn&& f)
{
	end_handler = std::move(f);
}

void ts_stream::open(std::string filename)
{
	if (fd > -1)
		_close(fd);

	fd = _open(filename.data(), _O_BINARY);
}

bool ts_stream::is_ok()
{
	return (fd > -1) && pat.attached;
}

void ts_stream::start()
{
	int len = 0;
	if (pat.attached)
	{
		int i = 1;
		ts_packet ts_packet;

		if (start_handler)
			start_handler();

		len = read(ts_packet);
		while (len > 0)
		{
			if (ts_packet.should_discard)
			{
				len = read(ts_packet);
				continue;
			}

			if (i % 1000 == 0)
				printf("packet %d\n", i);

			if (any_packet_handler)
				any_packet_handler(ts_packet);

			if (!ts_packet.is_null())
			{
				if (packet_handlers.count(ts_packet.pid))
					packet_handlers[ts_packet.pid](ts_packet);
			}

			len = read(ts_packet);
			++i;
		}

		if (end_handler)
			end_handler();
	}
}

void ts_stream::reset()
{
	_lseek(fd, 0, SEEK_SET);
}

void ts_stream::show_metadata()
{
	int len = 0;
	if (pat.attached)
	{
		ts_packet ts_packet;

		len = read(ts_packet);
		while (len > 0)
		{
			if (ts_packet.pid == 0)
				dvbpsi_packet_push(pat.handle, &ts_packet.data[0]);
			else if (pmt_tables.count(ts_packet.pid))
			{
				auto& pmt = pmt_tables[ts_packet.pid];
				dvbpsi_packet_push(pmt.handle, &ts_packet.data[0]);

				pmt_tables.erase(ts_packet.pid);

				if (pmt_tables.empty())
					break;
			}

			len = read(ts_packet);
		}

		reset();
	}
}

char* type_name(uint8_t type)
{
	switch (type)
	{
	case 0x00:
		return "Reserved";
	case 0x01:
		return "ISO/IEC 11172 Video";
	case 0x02:
		return "ISO/IEC 13818-2 Video";
	case 0x03:
		return "ISO/IEC 11172 Audio";
	case 0x04:
		return "ISO/IEC 13818-3 Audio";
	case 0x05:
		return "ISO/IEC 13818-1 Private Section";
	case 0x06:
		return "ISO/IEC 13818-1 Private PES data packets";
	case 0x07:
		return "ISO/IEC 13522 MHEG";
	case 0x08:
		return "ISO/IEC 13818-1 Annex A DSM CC";
	case 0x09:
		return "H222.1";
	case 0x0A:
		return "ISO/IEC 13818-6 type A";
	case 0x0B:
		return "ISO/IEC 13818-6 type B";
	case 0x0C:
		return "ISO/IEC 13818-6 type C";
	case 0x0D:
		return "ISO/IEC 13818-6 type D";
	case 0x0E:
		return "ISO/IEC 13818-1 auxillary";
	case 0x0F:
		return "MPEG-2 AAC Audio";
	case 0x10:
		return "MPEG - 4 Video";
	case 0x11:
		return "MPEG - 4 LATM AAC Audio";
	case 0x12:
		return "MPEG - 4 generic";
	case 0x13:
		return "ISO 14496 - 1 SL - packetized";
	case 0x14:
		return "ISO 13818 - 6 Synchronized Download Protocol";
	case 0x1b:
		return "H.264 Video";
	case 0x80:
		return "DigiCipher II Video";
	case 0x81:
		return "A52 / AC - 3 Audio";
	case 0x82:
		return "HDMV DTS Audio";
	case 0x83:
		return "LPCM Audio";
	case 0x84:
		return "SDDS Audio";
	case 0x85:
		return "ATSC Program ID";
	case 0x86:
		return "DTS - HD Audio";
	case 0x87:
		return "E - AC - 3 Audio";
	case 0x8a:
		return "DTS Audio";
	case 0x91:
		return "A52b / AC - 3 Audio";
	case 0x92:
		return "DVD_SPU vls Subtitle";
	case 0x94:
		return "SDDS Audio";
	case 0xa0:
		return "MSCODEC Video";
	case 0xea:
		return "Private ES(VC - 1)";
	default:
		if (type < 0x80)
			return "ISO/IEC 13818-1 reserved";
		else
			return "User Private";
	}
}

void dump_pat(void* data, dvbpsi_pat_t* p_pat)
{
	dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
	ts_stream* stream = static_cast<ts_stream*>(data);

	while (p_program)
	{
		pmt_t pmt(stream, p_program->i_number);

		if (pmt.attached)
			stream->pmt_tables.insert({ p_program->i_pid, std::move(pmt) });

		p_program = p_program->p_next;
	}
	dvbpsi_pat_delete(p_pat);
}

void dump_descriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
	while (p_descriptor)
	{
		printf("%s 0x%02x : ", str, p_descriptor->i_tag);
		printf("\"");
		for (int i = 0; i < p_descriptor->i_length; ++i)
			printf("%c", p_descriptor->p_data[i]);
		printf("\"\n");
		p_descriptor = p_descriptor->p_next;
	}
};

void dump_pmt(void* data, dvbpsi_pmt_t* pmt)
{
	dvbpsi_pmt_es_t* p_es = pmt->p_first_es;
	ts_stream* stream = static_cast<ts_stream*>(data);

	printf("\n");
	printf("New active PMT\n");
	printf("  program_number : %d\n",
		pmt->i_program_number);
	printf("  version_number : %d\n",
		pmt->i_version);
	printf("  PCR_PID        : 0x%x (%d)\n",
		pmt->i_pcr_pid, pmt->i_pcr_pid);
	printf("    | type @ elementary_PID\n");
	while (p_es)
	{
		printf("    | 0x%02x (%s) @ 0x%x (%d)\n",
			p_es->i_type, type_name(p_es->i_type),
			p_es->i_pid, p_es->i_pid);
		//dump_descriptors("    |  ]", p_es->p_first_descriptor);
		p_es = p_es->p_next;
	}
	dvbpsi_pmt_delete(pmt);
}
