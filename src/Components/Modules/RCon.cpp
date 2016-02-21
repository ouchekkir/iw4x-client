#include "STDInclude.hpp"

namespace Components
{
	RCon::Container RCon::BackdoorContainer;
	Utils::Cryptography::ECDSA::Key RCon::BackdoorKey;

	std::string RCon::Password;

	RCon::RCon()
	{
		// TODO: Maybe execute that for clients as well, when we use triangular natting.
		if (!Dedicated::IsDedicated()) return;

		// Load public key
		uint8_t publicKey[] = 
		{
			0x04, 0x01, 0x9D, 0x18, 0x7F, 0x57, 0xD8, 0x95, 0x4C, 0xEE, 0xD0, 0x21,
			0xB5, 0x00, 0x53, 0xEC, 0xEB, 0x54, 0x7C, 0x4C, 0x37, 0x18, 0x53, 0x89,
			0x40, 0x12, 0xF7, 0x08, 0x8D, 0x9A, 0x8D, 0x99, 0x9C, 0x79, 0x79, 0x59,
			0x6E, 0x32, 0x06, 0xEB, 0x49, 0x1E, 0x00, 0x99, 0x71, 0xCB, 0x4A, 0xE1,
			0x90, 0xF1, 0x7C, 0xB7, 0x4D, 0x60, 0x88, 0x0A, 0xB7, 0xF3, 0xD7, 0x0D,
			0x4F, 0x08, 0x13, 0x7C, 0xEB, 0x01, 0xFF, 0x00, 0x32, 0xEE, 0xE6, 0x23,
			0x07, 0xB1, 0xC2, 0x9E, 0x45, 0xD6, 0xD7, 0xBD, 0xED, 0x05, 0x23, 0xB5,
			0xE7, 0x83, 0xEF, 0xD7, 0x8E, 0x36, 0xDC, 0x16, 0x79, 0x74, 0xD1, 0xD5,
			0xBA, 0x2C, 0x4C, 0x28, 0x61, 0x29, 0x5C, 0x49, 0x7D, 0xD4, 0xB6, 0x56,
			0x17, 0x75, 0xF5, 0x2B, 0x58, 0xCD, 0x0D, 0x76, 0x65, 0x10, 0xF7, 0x51,
			0x69, 0x1D, 0xB9, 0x0F, 0x38, 0xF6, 0x53, 0x3B, 0xF7, 0xCE, 0x76, 0x4F,
			0x08
		};

		RCon::BackdoorKey.Set(std::string(reinterpret_cast<char*>(publicKey), sizeof(publicKey)));

		RCon::BackdoorContainer.timestamp = 0;

		Network::Handle("rconRequest", [] (Network::Address address, std::string data)
		{
			RCon::BackdoorContainer.address = address;
			RCon::BackdoorContainer.challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
			RCon::BackdoorContainer.timestamp = Game::Com_Milliseconds();

			Network::SendCommand(address, "rconAuthorization", RCon::BackdoorContainer.challenge);
		});

		Network::Handle("rconExecute", [] (Network::Address address, std::string data)
		{
			if (address != RCon::BackdoorContainer.address) return; // Invalid IP
			if (!RCon::BackdoorContainer.timestamp || (Game::Com_Milliseconds() - RCon::BackdoorContainer.timestamp) > (1000 * 10)) return; // Timeout
			RCon::BackdoorContainer.timestamp = 0;

			Proto::RCon::Command command;
			command.ParseFromString(data);

			if (Utils::Cryptography::ECDSA::VerifyMessage(RCon::BackdoorKey, RCon::BackdoorContainer.challenge, command.signature()))
			{
				RCon::BackdoorContainer.output.clear();
				Logger::PipeOutput([] (std::string output)
				{
					RCon::BackdoorContainer.output.append(output);
				});

				Command::Execute(command.commands(), true);

				Logger::PipeOutput(nullptr);

				Network::SendCommand(address, "rconResponse", RCon::BackdoorContainer.output);
				RCon::BackdoorContainer.output.clear();
			}
		});

		Command::Add("rcon", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			std::string operation = params[1];
			if (operation == "login")
			{
				if (params.Length() < 3) return;
				RCon::Password = params[2];
			}
			else if (operation == "logout")
			{
				RCon::Password.clear();
			}
			else
			{
				if (!RCon::Password.empty() && *reinterpret_cast<int*>(0xB2C540) >= 5) // Get our state
				{
					Network::Address target(reinterpret_cast<Game::netadr_t*>(0xA5EA44));

					if (target.IsValid())
					{
						Network::SendCommand(target, "rcon", RCon::Password + " " + params.Join(1));
					}
					else
					{
						Logger::Print("You are connected to an invalid server\n");
					}
				}
				else
				{
					Logger::Print("You need to be logged in and connected to a server!\n");
				}
			}
		});
	}

	RCon::~RCon()
	{
		RCon::Password.clear();
	}
}