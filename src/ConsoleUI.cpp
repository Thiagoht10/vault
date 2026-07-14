#include "ConsoleUI.hpp"

#include <unistd.h>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define ITALIC "\033[3m"
#define	BOLD "\033[1m"
#define RESET "\033[0m"

namespace
{
	void    consumeEscapeSequence(void)
	{
		char    seq;

		if (read(STDIN_FILENO, &seq, 1) != 1)
			return;
		if (seq == '[')
		{
			while (read(STDIN_FILENO, &seq, 1) == 1
					&& (seq < '@' || seq > '~'))
				;
		}
		else if (seq == 'O')
			read(STDIN_FILENO, &seq, 1);
	}
}

IUserInterface::InputResult ConsoleUI::readHiddenInput(SecureBuffer& pass,
		std::string prompt)
{
	SecureBuffer	password;
	std::cout << prompt << "\n" << "> ";
	std::cout.flush();

	if (!readHiddenLine(password))
	{
		std::cerr << "\n";
		return INPUT_INTERRUPTED;
	}
	
	if (password == "/cancel")
		return INPUT_CANCEL;
	
	pass = std::move(password);
	return INPUT_OK;
}

void    ConsoleUI::clearTerminal(void) const
{
	std::cout << "\033[2J" << "\033[3J" << "\033[H";
	std::cout.flush();
}

bool    ConsoleUI::getLine(std::string& input) const
{
	char    c;
	bool    completed;

	input.clear();
	completed = false;
	std::cout.flush();
	{
		TerminalEchoGuard guard(true);
		while (read(STDIN_FILENO, &c, 1) == 1)
		{
			if (c == '\n' || c == '\r')
			{
				completed = true;
				break;
			}

			if (c == 127 || c == '\b')
			{
				if (!input.empty())
				{
					input.erase(input.size() - 1);
					std::cout << "\b \b";
					std::cout.flush();
				}
				continue;
			}

			if (c == '\033')
			{
				consumeEscapeSequence();
				continue;
			}
			if (c < 32 || c == 127)
				continue;
			input += c;
			std::cout << c;
			std::cout.flush();
		}
	}
	std::cout << '\n';
	return completed || !input.empty();
}

bool    ConsoleUI::readHiddenLine(SecureBuffer& input) const
{
	char    c;
	bool    completed;

	input.erase();
	completed = false;
	{
		TerminalEchoGuard guard(true);
		while (true)
		{
			if (read(STDIN_FILENO, &c, 1) == 1)
			{
				if (c == '\n' || c == '\r')
				{
					completed = true;
					break;
				}
				if (c == 127 || c == '\b')
				{
					input.popByte();
					continue;
				}
				if (c == '\033')
				{
					consumeEscapeSequence();
					continue;
				}
				if (c < 32 || c == 127)
					continue;
				input.pushByte(static_cast<unsigned char>(c));
			}
			else
				return false;
		}
	}
	std::cout << '\n';
	return completed || !input.empty();
}

bool    ConsoleUI::waitEnterOrTimeout(int secunds) const
{
	fd_set          readFds;
	int             result;
	struct timeval  timeout;
	char            c;

	FD_ZERO(&readFds);
	FD_SET(STDIN_FILENO, &readFds);

	timeout.tv_sec = secunds;
	timeout.tv_usec = 0;

	result = select(STDIN_FILENO + 1, &readFds, NULL, NULL, &timeout);

	if (result == -1)
		return false;

	if (result == 0)
		return true;

	if (FD_ISSET(STDIN_FILENO, &readFds))
	{
		while (read(STDIN_FILENO, &c, 1) == 1)
		{
			if (c == '\n' || c == '\r')
				break;
		}
		return true;
	}
	return false;
}

IUserInterface::MenuInput   ConsoleUI::askMainMenuAction(
		const Message& message)
{
	std::string option;
	MenuInput   input;

	clearTerminal();
	if(!message.empty())
		showMessage(message);
	std::cout << "------------------" << std::endl;
	std::cout << "\nselect one option\n" << std::endl;
	std::cout << "1. add" << std::endl;
	std::cout << "2. show" << std::endl;
	std::cout << "3. delete" << std::endl;
	std::cout << "4. edit" << std::endl;
	std::cout << "5. change master password" << std::endl;
	std::cout << "0. exit\n";
	input.result = INPUT_OK;
	input.action = ACTION_INVALID;

	std::cout << "\n> ";
	if (!getLine(option))
	{
		std::cerr << "\n";
		input.result = INPUT_INTERRUPTED;
		return input;
	}

	if (option == "0")
	{
		clearTerminal();
		input.action = ACTION_EXIT;
	}
	else if (option == "1")
	{
		clearTerminal();
		input.action = ACTION_ADD;
	}
	else if (option == "2")
	{
		clearTerminal();
		input.action = ACTION_SHOW;
	}
	else if (option == "3")
	{
		clearTerminal();
		input.action = ACTION_DELETE;
	}
	else if (option == "4")
	{
		clearTerminal();
		input.action = ACTION_EDIT;
	}
	else if (option == "5")
	{
		clearTerminal();
		input.action = ACTION_CHANGE_PASSWORD;
	}
	return input;
}

IUserInterface::InputResult ConsoleUI::askPassWord(SecureBuffer& pass,
		std::string prompt)
{
	return readHiddenInput(pass, prompt);
}

void    ConsoleUI::showEntryList(const Vault& vault) const
{
	std::stringstream   ss;

	for (size_t i = 0; i < vault.size(); i++)
	{
		const Entry& entry = vault.getEntry(i);

		if (i > 0)
			std::cout << "\n" << "------------------" << std::endl;
		std::cout << "\n[" << i << "]" << std::endl;
		std::cout << "service: " << entry.getService() << std::endl;
	}
}

void    ConsoleUI::showEntryDetais(const Entry& entry) const
{
	std::cout << "\n" << "------------------" << std::endl;
	std::cout << "service: ";
	std::cout.write(reinterpret_cast<const char*>(entry.getService()),
			entry.getServiceSize()) << "\n";
	std::cout << "username: ";
	std::cout.write(reinterpret_cast<const char*>(entry.getUsername()),
			entry.getUserNameSize()) << "\n";
	std::cout << "password: ********" << std::endl;
	std::cout << "------------------" << std::endl;
}

IUserInterface::InputResult ConsoleUI::showPasswordTemporarily(const Entry& entry) const
{
	std::string tmp;

	std::cout << "\033[2J\033[H";

	std::cout << "password: ";
	std::cout.write(reinterpret_cast<const char*>(entry.getPassword()),
			entry.getPasswordSize()) << "\n";

	std::cout << "\nthis message will keep visible for 5 secunds";
	std::cout << "\npress Enter to continue...";
	std::cout.flush();

	if (!waitEnterOrTimeout(5))
		return INPUT_INTERRUPTED;

	std::cout << "\033[2J\033[H";
	std::cout.flush();
	return INPUT_OK;
}

IUserInterface::InputResult ConsoleUI::showEntryTemporarily(const Entry& entry) const
{
	InputResult result;
	std::string input;

	std::cout << "\033[?1049h"; // entra na tela temporaria
	std::cout << "\033[2J\033[H";

	showEntryDetais(entry);
	std::cout << "\ndo you want to reveal the password?" << std::endl;
	std::cout << "Y to yes\n> ";
	std::cout.flush();
	if (!getLine(input))
		result = INPUT_INTERRUPTED;
	else if (input == "Y" || input == "y")
		result = showPasswordTemporarily(entry);
	else
		result = INPUT_OK;

	std::cout << "\033[2J\033[H";
	std::cout << "\033[?1049l";
	std::cout.flush();
	return result;
}

IUserInterface::InputOutcome ConsoleUI::askEntryIndex(size_t& index,
		const Vault& vault) const
{
	std::stringstream   ss;
	std::string         input;
	InputOutcome        outcome;

	outcome.result = INPUT_OK;

	std::cout << "\n" << "------------------" << "\n" << std::endl;
	std::cout << "please, select an index" << std::endl;
	std::cout << ITALIC << "type /cancel to cancel" << RESET << "\n> ";

	if (!getLine(input))
	{
		std::cerr << "\n";
		outcome.result = INPUT_INTERRUPTED;
		return outcome;
	}

	if (input.empty())
	{
		outcome.result = INPUT_INVALID;
		outcome.message.set("index cannot be empty", Message::ERROR);
		return outcome;
	}

	if (input == "/cancel")
	{
		outcome.result = INPUT_CANCEL;
		return outcome;
	}
	
	for (size_t i = 0; input[i]; i++)
	{
		if (input[i] < '0' || input[i] > '9')
		{
			outcome.result = INPUT_INVALID;
			outcome.message.set("index must be a number", Message::ERROR);
			return outcome;
		}
	}
	ss << input;
	ss >> index;

	if (!vault.isValidIndex(index))
	{
		outcome.result = INPUT_INVALID;
		outcome.message.set("index is out of range", Message::ERROR);
		return outcome;
	}
	return outcome;
}

void    ConsoleUI::showError(std::string error) const
{
	std::cerr << "\n" << RED << error << RESET << "\n" << std::endl;
}

IUserInterface::InputOutcome ConsoleUI::askNewEntry(Entry& entry)
{
	SecureBuffer    password;
	SecureBuffer    checkPass;
	SecureString    tmp;
	InputResult     result;
	InputOutcome    outcome;

	outcome.result = INPUT_OK;

	std::cout << "please, insert datas:" << std::endl;
	std::cout << ITALIC << "type /cancel to cancel\n" << RESET<< std::endl;

	while(1)
	{
		tmp.erase();
		std::cout << "service: \n" << "> ";
		std::cout.flush();
		if (!tmp.readBytes())
		{
			std::cerr << "\n";
			outcome.result = INPUT_INTERRUPTED;
			return outcome;
		}
		if(tmp.empty())
		{
			showError("can't have empty inputs");
			continue ;
		}
		if(tmp == "/cancel")
		{
			outcome.result = INPUT_CANCEL;
			return outcome;
		}

		entry.setService(tmp.data(), tmp.size());
		tmp.erase();
		break;
	}

	while (1)
	{
		tmp.erase();
		std::cout << "username: \n" << "> ";
		std::cout.flush();
		if (!tmp.readBytes())
		{
			std::cerr << "\n";
			outcome.result = INPUT_INTERRUPTED;
			return outcome;
		}
		if (tmp.empty())
		{
			showError("can't have empty inputs");
			continue ;
		}
		if(tmp == "/cancel")
		{
			outcome.result = INPUT_CANCEL;
			return outcome;
		}

		entry.setUsername(tmp.data(), tmp.size());
		tmp.erase();
		break;
	}
	
	while (1)
	{
		bool    match = false;

		for (size_t attempt = 0; attempt < 3 && !match; attempt++)
		{
			password.erase();
			checkPass.erase();

			result = readHiddenInput(password, "password:");
			if (result != INPUT_OK)
			{
				outcome.result = result;
				return outcome;
			}

			if (password.empty())
			{
				showError("can't have empty inputs");
				continue;
			}

			result = readHiddenInput(checkPass,
					"\nplease, confirm your password");
			if (result != INPUT_OK)
			{
				outcome.result = result;
				return outcome;
			}

			if(password == checkPass)
				match = true;
			else
			{
				showError("password not match");
				if (attempt < 3)
					std::cerr << "you have " << (3 - attempt - 1) << " attempts\n" << std::endl;
				password.erase();
				checkPass.erase();
			}
			if (attempt + 1 == 3 && !match)
			{
				outcome.result = INPUT_INVALID;
				outcome.message.set("password confirmation failed",
						Message::ERROR);
				return outcome;
			}
		}
		entry.setPassword(password.data(), password.size());
		password.erase();
		checkPass.erase();
		break;
	}
	return outcome;
}

IUserInterface::ConfirmationInput ConsoleUI::askConfirmation(const Entry& entry) const
{
	std::string         option;
	ConfirmationInput   input;

	input.result = INPUT_OK;
	input.confirmed = false;
	std::cout << "\nare you sure you want to delete this credential?\n\n";
	std::cout << BOLD << "service: " << entry.getService() << RESET << std::endl;
	std::cout << BOLD << "username: " << entry.getUsername() << RESET << std::endl;
	std::cout << "\nY to yes | N to not\n" << std::endl;

	while (option != "N" && option != "n" && option != "Y" && option != "y")
	{
		std::cout << "> ";
		if (!getLine(option))
		{
			std::cerr << "\n";
			input.result = INPUT_INTERRUPTED;
			return input;
		}
		if (option != "N" && option != "n" && option != "Y" && option != "y")
			showError("invalid option");
	}

	if (option == "N" || option == "n")
		return input;

	input.confirmed = true;
	return input;
}

void    ConsoleUI::showMessage(const Message& message) const
{
	const char* color = RESET;

	if (message.type() == Message::SUCCESS)
		color = GREEN;
	else if (message.type() == Message::INFO)
		color = YELLOW;
	else if (message.type() == Message::ERROR)
		color = RED;
	std::cout << color << message.text() << RESET << "\n" << std::endl;
}


IUserInterface::InputOutcome ConsoleUI::askEditEntry(SecureBuffer& pass,
		SecureBuffer& usr)
{
	SecureBuffer    password;
	SecureBuffer    confirmPassword;
	SecureBuffer	username;
	bool            match;
	IUserInterface::InputResult	result;
	InputOutcome    outcome;

	outcome.result = INPUT_OK;

	std::cout << "\nput your new username\n";
	std::cout << ITALIC << "put enter for use the same" << RESET << "\n> ";
	std::cout.flush();
	if (!username.readBytes())
	{
		std::cerr << "\n";
		outcome.result = INPUT_INTERRUPTED;
		return outcome;
	}
	if (username == "/cancel")
	{
		outcome.result = INPUT_CANCEL;
		return outcome;
	}

	match = false;
	for (size_t attempt = 0; attempt < 3 && !match; attempt++)
	{
		password.erase();
		confirmPassword.erase();

		result = readHiddenInput(password, "put your new password");
		if (result != INPUT_OK)
		{
			outcome.result = result;
			return outcome;
		}
		
		if (password.empty())
		{
			showError("can't have empty inputs");
			continue;
		}

		result = readHiddenInput(confirmPassword, "confirm your new password");
		if (result != INPUT_OK)
		{
			outcome.result = result;
			return outcome;
		}

		if(password == confirmPassword)
			match = true;
		else
		{
			showError("password not match");
			if (attempt < 3)
				std::cerr << "you have " << (3 - attempt - 1) << " attempts\n" << std::endl;
			password.erase();
			confirmPassword.erase();
		}

		if (attempt + 1 == 3 && !match)
		{
			outcome.result = INPUT_INVALID;
			outcome.message.set("password confirmation failed",
					Message::ERROR);
			return outcome;
		}
	}
	usr = std::move(username);
	pass = std::move(password);
	confirmPassword.erase();
	return outcome;
}
