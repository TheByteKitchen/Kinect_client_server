//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "pch.h"
#include "KinectColorDepthClient.h"
#include "KinectData.h"
#include "runlengthcodec.h"

using namespace TheByteKitchen::_3D_TV;
using namespace rlecodec;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

#define RLECOMPRESSION	

const wstring conn_succ_c(L"Color channel connected\n");
const wstring conn_fail_c(L"Color channel connection failed.\n3D-TV could not reach the remote host at ip address 192.168.0.20,\nor the KinectColorDepthServerApp is not running (at 192.168.0.20).\n\n");

const wstring conn_succ_d(L"Depth channel connected\n");
const wstring conn_fail_d(L"Depth channel connection failed.\n3D-TV could not reach the remote host at ip address 192.168.0.20,\nor the KinectColorDepthServerApp is not running (at 192.168.0.20).\n\n");

String^ start_mess_c(L"Start Sending Color");
const wstring send_succ_c(L"Start Sending Color message sent\n");
const wstring send_fail_c(L"Sending Start Color Message failed.\n\n");

String^ start_mess_d(L"Start Sending Depth");
const wstring send_succ_d(L"Start Sending Depth message sent\n");
const wstring send_fail_d(L"Sending Start Depth Message failed.\n\n");

const wstring spec_fail_c(L"3D-TV received an incorrect Color data length specification.\nData processing has been stopped.\nPlease check the KinectColorDepthServerApp console.\n\n");
const wstring data_fail_c(L"3D-TV received an incorrect Color data length.\nData processing has been stopped.\nPlease check the KinectColorDepthServerApp console.\n\n");

const wstring spec_fail_d(L"3D-TV received an incorrect Depth data length specification.\nData processing has been stopped.\nPlease check the KinectColorDepthServerApp console.\n\n");
const wstring data_fail_d(L"3D-TV received an incorrect Color data length.\nData processing has been stopped.\nPlease check the KinectColorDepthServerApp console.\n\n");


KinectColorDepthClient::KinectColorDepthClient(LoggingEventFunc lmf, FrameReceivedCallback frc) :
CurrentState(ReceiveState::INACTIVE), dataLoadCnt(0), LogMessage(lmf), FrameReceived(frc) { }

KinectColorDepthClient::KinectColorDepthClient() { }

template<typename T>
inline bool try_catch(const task<T>& pt, ReceiveState& state, const LoggingEventFunc& log, const wstring& success, const wstring& failure)
{
	try
	{
		pt.get();
		if (success != L"")
			log(success);

		return true;
	}
	catch (Exception^)
	{
		state = ReceiveState::EXCEPTION;
		if (failure != L"")
			log(failure);

		return false;
	}
	catch (const task_canceled&)
	{
		state = ReceiveState::EXCEPTION;

		// Swallow exception and signal the loop
		return false;
	}
}

inline task<bool> connection(StreamSocket^ sock, HostName^ host, String^ port, ReceiveState& state, const LoggingEventFunc& log, const wstring& success, const wstring& fail)
{
	return task<void>(sock->ConnectAsync(host, port, SocketProtectionLevel::PlainSocket)).then([&state, log, success, fail](task<void> pt)
	{
		return try_catch(pt, state, log, success, fail);
	});
}

inline task<bool> start_message(StreamSocket^ sock, String^ msg, ReceiveState& state, const LoggingEventFunc& log, const wstring& success, const wstring& failure)
{
	return task<unsigned int>([sock, msg]()
	{
		DataWriter^ writer = ref new DataWriter(sock->OutputStream);

		writer->WriteString(msg);

		// Write the locally buffered data to the network.
		return task<unsigned int>(writer->StoreAsync());

	}).then([&state, log, success, failure](task<unsigned int> pt)
	{
		return try_catch(pt, state, log, success, failure);
	});
}

#ifdef RLECOMPRESSION

template<typename Cod>
struct rcv_context
{
	DataReader^ reader;
	Cod& codec;
	unsigned char* spec;
	unsigned int spec_size;
	unsigned char* data;
	LoggingEventFunc& log;
	wstring const& spec_fail;
	wstring const & data_fail;
};

template<typename Cod>
inline task<bool> receive_frame(rcv_context<Cod>& c, ReceiveState& state)
{
	return create_task(c.reader->LoadAsync(c.spec_size)).then([&c, &state](unsigned int size)
	{
		if (size != c.spec_size)
		{
			state = ReceiveState::EXCEPTION;

			c.log(c.spec_fail);

			// The underlying socket was closed before we were able to read the whole data.
			cancel_current_task();
		}
		else
		{
			// Size spec. is ok
			c.reader->ReadBytes(ArrayReference<unsigned char>(c.spec, size));
		}

		// Load data into buffer
		return create_task(c.reader->LoadAsync(c.codec.size())).then([&c, &state](unsigned int actualLength)
		{
			if (actualLength != c.codec.size())
			{
				state = ReceiveState::EXCEPTION;

				c.log(c.data_fail);

				// The underlying socket was closed before we were able to read the whole data.
				cancel_current_task();
			}
			else
			{
				// Actual length is ok
				c.reader->ReadBytes(ArrayReference<unsigned char>(c.data, actualLength));
			}
		});
	}).then([&c, &state](task<void> tb)
	{
		// catch any task_canceled exceptions. No additional messages required
		return try_catch(tb, state, c.log, L"", L"");
	});
}

#else

struct rcv_context
{
	DataReader^ reader;
	unsigned char* data;
	LoggingEventFunc& log;
	wstring const& spec_fail;
	wstring const & data_fail;
};


inline task<bool> receive_frame(rcv_context& c)
{
	return create_task(c.reader->LoadAsync(sizeof(unsigned int))).then([&c](unsigned int size)
	{
		if (size != sizeof(unsigned int))
		{
			auto s = c.spec_fail + size.ToString()->Data() + L" Task canceled\n";
			c.log(s);

			// The underlying socket was closed before we were able to read the whole data.
			cancel_current_task();
		}

		// Still here, size spec. is ok
		size = c.reader->ReadUInt32();

		// Load data into buffer
		return create_task(c.reader->LoadAsync(size)).then([&c, size](unsigned int actualLength)
		{
			if (actualLength != size)
			{
				auto s = c.data_fail + actualLength.ToString()->Data() + L" Task canceled\n";
				c.log(s);

				// The underlying socket was closed before we were able to read the whole data.
				cancel_current_task();
			}

			// still here, actual length is ok
			c.reader->ReadBytes(ArrayReference<unsigned char>(c.data, actualLength));
		});
	}).then([&c](task<void> tb)
	{
		// catch any task_canceled exceptions
		return try_catch(tb, c.log, L"", L"");
	});
}

#endif

void KinectColorDepthClient::StartReceiveAsync(StreamSocket^ colorSocket, StreamSocket^ depthSocket)
{
	auto ReceiveAsyncTask = task<void>([this, colorSocket, depthSocket]()
	{
		CurrentState = ReceiveState::INACTIVE;

		LogMessage(L"Initializing Receiving and Processing\n");

		ColorByteSize = KinectData::Current()->ColorByteSize();
		DepthByteSize = KinectData::Current()->DepthByteSize();

		colordata = vector<unsigned char>(ColorByteSize / sizeof(unsigned char));
		depthdata = vector<unsigned short>(DepthByteSize / sizeof(unsigned short));

#pragma region connection
		// scope, to manage life cycle of locals for connecting
		{
			CurrentState = ReceiveState::CONNECTING;

			LogMessage(L"Trying to connect to the server\n");

			HostName^ hostName = ref new HostName(HOSTNAME);

			auto hns = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
			for (auto hn : hns)
			{
				if (hn->IsEqual(ref new HostName(L"192.168.0.20")))
				{
					CurrentState = ReceiveState::EXCEPTION;

					LogMessage(L"Your PC has a network adapter with ip address 192.168.0.20.\n3D-TV cannnot run on a PC with ip address 192.168.0.20.\n");

					break;
				}
			}

			if (CurrentState == ReceiveState::CONNECTING)
			{
				auto connection_join =
					connection(depthSocket, hostName, DEPTHPORT, CurrentState, LogMessage, conn_succ_d, conn_fail_d) &&
					connection(colorSocket, hostName, COLORPORT, CurrentState, LogMessage, conn_succ_c, conn_fail_c);

				auto results = connection_join.get();
				if (!(results[0] && results[1])) CurrentState = ReceiveState::EXCEPTION;
			}
		}
#pragma endregion connection

#pragma region start message
		if (CurrentState == ReceiveState::CONNECTING)
		{
			auto send_join =
				start_message(depthSocket, start_mess_d, CurrentState, LogMessage, send_succ_d, send_fail_d) &&
				start_message(colorSocket, start_mess_c, CurrentState, LogMessage, send_succ_c, send_fail_c);

			auto results = send_join.get();
			if (!(results[0] && results[1])) CurrentState = ReceiveState::EXCEPTION;
		}
#pragma endregion start message

		if (CurrentState == ReceiveState::CONNECTING)
		{
			// Explicit state transition
			CurrentState = ReceiveState::RECEIVING;

			LogMessage(L"Receiving and processing data\n");

			// DataReaders
			DataReader^ m_depthReader = ref new DataReader(depthSocket->InputStream);
			m_depthReader->ByteOrder = ByteOrder::LittleEndian;
			DataReader^ m_colorReader = ref new DataReader(colorSocket->InputStream);
			m_colorReader->ByteOrder = ByteOrder::LittleEndian;

#ifdef RLECOMPRESSION
			// Compression codecs
			runlengthcodec<unsigned short> m_depthCodec(KinectData::Current()->DepthByteSize() / sizeof(unsigned short));
			runlengthcodec<unsigned char> m_colorCodec(KinectData::Current()->ColorByteSize() / sizeof(unsigned char));

			rcv_context<runlengthcodec<unsigned short>> dc = { m_depthReader, m_depthCodec, reinterpret_cast<unsigned char*>(&(m_depthCodec.channelsize)), sizeof(m_depthCodec.channelsize), m_depthCodec.data().get(),
				LogMessage, spec_fail_d, data_fail_c };

			rcv_context<runlengthcodec<unsigned char>> cc = { m_colorReader, m_colorCodec, reinterpret_cast<unsigned char*>(&(m_colorCodec.channelsize)), sizeof(m_colorCodec.channelsize), m_colorCodec.data().get(),
				LogMessage, spec_fail_c, data_fail_c };

#else
			rcv_context dc = { m_depthReader, (unsigned char*)depthdata.data(), LogMessage, spec_fail_d, data_fail_c };
			rcv_context cc = { m_colorReader, colordata.data(), LogMessage, spec_fail_c, data_fail_c };
#endif
			// Start a receiving loop to receive data
			while (CurrentState == ReceiveState::RECEIVING)
			{
				auto receive_join = receive_frame(dc, CurrentState) && receive_frame(cc, CurrentState);
				auto results = receive_join.get();

				if (results[0] && results[1])
				{
#ifdef RLECOMPRESSION
					// decompress and copy data
					auto dcdtask = create_task([this, &m_depthCodec]()
					{
						m_depthCodec.decode(depthdata);
						memcpy(KinectData::Current()->DepthDataPtr(), (unsigned char*)depthdata.data(), DepthByteSize);
					});

					auto dcctask = create_task([this, &m_colorCodec]()
					{
						m_colorCodec.decode(colordata);
						memcpy(KinectData::Current()->ColorDataPtr(), colordata.data(), ColorByteSize);
					});

					auto decode_join = dcctask && dcdtask;

					decode_join.get();
#else
					memcpy(KinectData::Current()->DepthDataPtr(), (unsigned char*)depthdata.data(), DepthByteSize);
					memcpy(KinectData::Current()->ColorDataPtr(), colordata.data(), ColorByteSize);
#endif
					// signal owner that data has been received and is ready for use
					FrameReceived();
				}
				else
				{
					if (CurrentState != ReceiveState::EXCEPTION)
						CurrentState = ReceiveState::INACTIVE;

					LogMessage(L"3D-TV was disconnected from server.\n\n");
				}
			}
		}

		LogMessage(L"Stopped receiving and processing data.");
	});
}

void KinectColorDepthClient::StopReceiveAsync()
{
	CurrentState = ReceiveState::INACTIVE;
}


