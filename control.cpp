#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <mysql.h>
#include <colours.h>
#include <cctype> // include the cctype library for toupper
#include <filesystem>

using namespace std;

MYSQL_RES* res;
MYSQL_ROW row;
MYSQL_FIELD *field;
string table_name = "classic", accname, accselect, nickname, x, xselect, y, yselect, level, levelselect, player, playerselect, guildname, guildnameselect, branding = "Zed Server 1.39.23 Control Console";
int menu = 0; //0 = Main Menu, 1 = User Menu, 2 = Misc Menu
int returnmenu = 0;
int recentaccounts = 0;




string getTableName() {
	if (table_name != "classic") {
		cout << "Enter table name (default: classic): ";
		getline(cin, table_name);
		if (table_name.empty()) {
			table_name = "classic";
		}
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input stream
    return table_name;
}




string getInput(const string& prompt, string& input) {
    cout << prompt;
    string temp;
    getline(cin, temp);
    if (!temp.empty()) {
        input = temp;
    }
    while (input.empty()) {
        cout << "Input cannot be blank. Please enter a valid value: ";
        getline(cin, input);
    }
    return input;
}

string getAccname(string& accselect) {
	string prompt = "";
	if (menu > 1) {
		prompt = "Enter account";
		if (!accselect.empty()) {
			prompt += " (default: " + accselect + ")";
		}
		prompt += ": ";
	}
    return getInput(prompt, accselect);
}

string getGuildname(string& guildnameselect) {
    string prompt = "Enter guild";
    if (!guildnameselect.empty()) {
        prompt += " (default: " + guildnameselect + ")";
    }
    prompt += ": ";
    return getInput(prompt, guildnameselect);
}

string getGuildplayer(string& playerselect) {
    string prompt = "Enter player's guild name";
    if (!playerselect.empty()) {
        prompt += " (default: " + playerselect + ")";
    }
    prompt += ": ";
    return getInput(prompt, playerselect);
}

void clearConsole(int numRows) {
    for (int i = 0; i < numRows; i++) {
        std::cout << std::endl;
    }
}

bool checkFileExistence(const string& folder, const string& value) {
    return std::filesystem::exists("data/" + folder + "/" + value);
}

void ControlDBUpdates(MYSQL* conn, const string& accname, string property_name) {
	int modification;
    if (property_name == "accname") {
        modification = 1;
    } else if (property_name == "nickname") {
        modification = 2;
    } else if (property_name == "x") {
        modification = 3;
    } else if (property_name == "y") {
        modification = 4;
    } else if (property_name == "level") {
        modification = 5;
    } else if (property_name == "maxhp") {
        modification = 6;
    } else if (property_name == "hp") {
        modification = 7;
    } else if (property_name == "rupees") {
        modification = 8;
    } else if (property_name == "arrows") {
        modification = 9;
    } else if (property_name == "bombs") {
        modification = 10;
    } else if (property_name == "glovepower") {
        modification = 11;
    } else if (property_name == "swordpower") {
        modification = 12;
    } else if (property_name == "shieldpower") {
        modification = 13;
    } else if (property_name == "headimg") {
        modification = 14;
    } else if (property_name == "swordimg") {
        modification = 15;
    } else if (property_name == "shieldimg") {
        modification = 16;
    } else {
        modification = 0; //invalid input
    }
	string query = "INSERT INTO updates (accname, modification) VALUES ('" + accname + "', '" + std::to_string(modification) + "')";
	if (mysql_query(conn, query.c_str())) {
		cout << "Error: could not execute query" << endl;
	} else {
		//cout << "Successfully added " << accname << " changes to the updates queue." << endl;
	}
	//mysql_free_result(res);
}

void selectFieldsFromTable(MYSQL* conn, const std::string& table_name, const std::string& accname, const std::vector<std::string>& fields, std::vector<std::string>& values) {
    std::string query = "SELECT ";
    for (size_t i = 0; i < fields.size(); i++) {
        query += fields[i];
        if (i != fields.size() - 1) {
            query += ",";
        }
    }
    query += " FROM " + table_name + " WHERE accname='" + accname + "'";
    
    mysql_query(conn, query.c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    if (mysql_num_rows(res) == 0) {
        std::cout << "No player with the specified accname found." << std::endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    values.resize(fields.size());
    for (size_t i = 0; i < fields.size(); i++) {
        values[i] = row[i];
    }
    if (res != nullptr) {
		//Clear res, but only when it is used!
		mysql_free_result(res); 
	}
}

//Player Functions
void updatePlayerProperty(MYSQL* conn, const string& table_name, const string& accname, const string& property_name, const string& message) {
    cout << YELLOW << message << WHITE;
    string value;
	string confirm;
	string folder = "";
    getline(cin, value);
	string capitalized_name = string(1, toupper(property_name[0])) + property_name.substr(1);
    if (value.empty()) {
		cout << capitalized_name << " not updated." << endl;
	} else {
		bool abort = false;
		if (property_name == "headimg") {
			folder = "heads";
		} else if (property_name == "swordimg") {
			folder = "swords";
		} else if (property_name == "shieldimg") {
			folder = "shields";
		}
		
		if (!folder.empty() && !checkFileExistence(folder, value)) {
			abort = true;
		}
		if (abort == true) {
				cout << "Image not found! Are you sure you want to assign an invalid " << capitalized_name << " to " << nickname << "? (y/n): ";
				cin >> confirm;
				if (confirm == "y") {
					abort = false;
				}
		}
		if (abort == true) {
		} else {
			mysql_query(conn, ("UPDATE " + table_name + " SET " + property_name + "='" + value + "' WHERE accname='" + accname + "'").c_str());
			ControlDBUpdates(conn, accname, property_name);
			cout << capitalized_name << " updated to " << value << "." << endl;
		}
	}
}

void updatePlayerLevel(MYSQL* conn, const string& table_name, const string& accname) {
     mysql_query(conn, ("SELECT nickname, x, y, level FROM " + table_name + " WHERE accname='" + accname + "'").c_str());
    res = mysql_store_result(conn);
    if (mysql_num_rows(res) == 0) {
        cout << "No player with the specified accname found." << endl;
        return;
    }
    row = mysql_fetch_row(res);
    nickname = row[0];
    x = row[1];
    y = row[2];
    level = row[3];
	
	cout << CYAN << "\n" << branding << "\n\n" << WHITE << "Change Level\n" << endl; 
	cout << GREEN << "   Account Name" << WHITE << ": " << MAGENTA << accname << "\n";
	cout << GREEN << "   Nick Name" << WHITE << ": " << MAGENTA << nickname << "\n";
	cout << GREEN << "   X" << WHITE << ": " << MAGENTA << x << "\n";
	cout << GREEN << "   Y" << WHITE << ": " << MAGENTA << y << "\n";
	cout << GREEN << "   Level" << WHITE << ": " << MAGENTA << level << "\n" << WHITE;
	const auto [ROWS,COLS] = get_terminal_size();
	clearConsole(ROWS - 3);
	updatePlayerProperty(conn, table_name, accname, "level", "Enter new level (press enter to keep current level): ");
	updatePlayerProperty(conn, table_name, accname, "x", "Enter X coordinate (press enter to keep current coordinate): ");
	updatePlayerProperty(conn, table_name, accname, "y", "Enter Y coordinate (press enter to keep current coordinate): ");
}

//Guild Functions
void addGuildMember(MYSQL* conn, const string& accname) {
	player = getGuildplayer(playerselect);
	guildname = getGuildname(guildnameselect);
	string query = "INSERT INTO guilds (accname, player, rank, guildname) VALUES ('" + accname + "', '" + player + "', '0', '" + guildname + "')";
	if (mysql_query(conn, query.c_str())) {
		cout << "Error: could not execute query" << endl;
	} else {
		cout << "Successfully added " << player << " to the guild!" << endl;
	}
}			
				
void removeGuildMember(MYSQL* conn, const string& accname) {
    // Retrieve the corresponding player and guildname fields from the guilds table
    string query = "SELECT player, guildname FROM guilds WHERE accname='" + accname + "'";
    if (mysql_query(conn, query.c_str())) {
        //finish_with_error(conn);
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (mysql_num_rows(res) == 0) {
        cout << "No guild member with the specified account name found." << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string player = row[0];
    string guildname = row[1];

    // Confirm with the user before proceeding with the removal
    string confirm;
    cout << "Are you sure you want to remove " << player << " from " << guildname << "? (y/n): ";
    cin >> confirm;
    if (confirm != "y") {
        return;
    }

    // Remove the row with the specified accname from the guilds table
    query = "DELETE FROM guilds WHERE accname='" + accname + "'";
    if (mysql_query(conn, query.c_str())) {
        //finish_with_error(conn);
    }
    cout << "Guild member " << player << " has been removed from " << guildname << "." << endl;
}




void selectActiveAccounts(MYSQL* conn) {
	const auto [ROWS,COLS] = get_terminal_size();
	table_name = getTableName();
	recentaccounts = 0;
	string query = "SELECT accname, nickname FROM " + table_name + " ORDER BY onlinetime DESC LIMIT " + std::to_string(ROWS+ROWS);
	if (mysql_query(conn, query.c_str())) {
		cout << "Error executing query: " << mysql_error(conn) << endl;
	}
	res = mysql_store_result(conn);
	int count = mysql_num_rows(res);
	if (count == 0) {
		cout << "No entries found." << endl;
	} else {
		clearConsole(ROWS);
		cout << CYAN << "\n" << branding << "\n\n" << WHITE << "Most Active Accounts\n" << endl; 
		while (row = mysql_fetch_row(res)) {
			if (std::string(row[0]).find("Guest") == std::string::npos) {
				if (std::string(row[0]).find("Deleteme") == std::string::npos) {
					if (std::string(row[0]).find("DELETEK") == std::string::npos) {
						//cout << CYAN << row[0] << WHITE << " is known in-game as " << CYAN << row[1] << WHITE << ".\n";
						if (std::string(row[0]).find("admin") == std::string::npos) {
							cout << GREEN << "   " << row[0] << WHITE << " AKA " << MAGENTA << row[1] << "\n";
							recentaccounts = recentaccounts + 1;
						}
					}
				}
			}
		}
		
	}
	cout << "\n";
	cout << RED << "   Enter Desired Account " << WHITE << std::endl; 
	clearConsole(ROWS - 6 - recentaccounts);
	cout << YELLOW << "Input: " << WHITE; 
	accname = getAccname(accselect);
}






int main() {
    MYSQL* conn;
    ifstream file("config/mysql.txt");
    string server, username, password, database, socket_path;

    // Read login details from file
	if (file.is_open()) {
		string line;
		auto trim_left = [](string& str) {
			str.erase(0, str.find_first_not_of(" "));
		};
		while (getline(file, line)) {
			if (line.substr(0, 7) == "server=") {
				server = line.substr(7);
				trim_left(server);
			} else if (line.substr(0, 9) == "username=") {
				username = line.substr(9);
				trim_left(username);
			} else if (line.substr(0, 9) == "password=") {
				password = line.substr(9);
				trim_left(password);
			} else if (line.substr(0, 9) == "database=") {
				database = line.substr(9);
				trim_left(database);
			} else if (line.substr(0, 7) == "socket=") {
				socket_path = line.substr(7);
				trim_left(socket_path);
			}
		}
		file.close();
	} else {
		cerr << "Error: Unable to open file!" << endl;
		return 1;
	}
    // Connect to MariaDB using socket
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server.c_str(), username.c_str(), password.c_str(), database.c_str(), 0, socket_path.c_str(), 0)) {
        cerr << "Error: Unable to connect to MariaDB: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return 1;
    }
    cout << "Connected to MariaDB successfully!" << endl;
    // Display menu options
    int choice = -1;
    while (choice != 0) {
		//0 = Main Menu, 1 = User Menu, 2 = Misc Menu
		const auto [ROWS,COLS] = get_terminal_size();
		if (menu == 0) {
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "Main Menu\n" << endl; 
			cout << GREEN << "   1." << WHITE << " User Management\n";
			cout << GREEN << "   2." << WHITE << " Miscellaneous\n";
			cout << GREEN << "   0." << WHITE << " Exit\n\n";
			cout << RED << "   Select Menu Number [0-2]\n" << WHITE << std::endl; 
			clearConsole(ROWS - 10);
			cout << YELLOW << "Input: " << WHITE; 
			cin >> choice;
			switch (choice) {
				case 1: {
					menu = 1;
					break;
				}
				case 2: {
					menu = 2;
					break;
				}
				case 0: {
					cout << YELLOW << "Goodbye! Thanks for using the " << branding << "!" << WHITE << endl;
					clearConsole(ROWS - 1);
					break;
				}
			}
		}
		
		if (menu == 1) {
			if (accname.empty()) {
				selectActiveAccounts(conn);
			}
			
			mysql_query(conn, ("SELECT nickname, x, y, level, maxhp, hp, rupees, arrows, bombs, glovepower, swordpower, shieldpower, headimg, swordimg, shieldimg FROM " + table_name + " WHERE accname='" + accname + "'").c_str());
			res = mysql_store_result(conn);

			if (mysql_num_rows(res) == 0) {
				cout << "No player with the specified accname found." << endl;
			}
			row = mysql_fetch_row(res);
			nickname = row[0];
			level = row[1];
			
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "User Management\n" << endl; 
			
			cout << GREEN << "   Account Name" << WHITE << ": " << MAGENTA << accname << "\n";
			cout << GREEN << "   Nick Name" << WHITE << ": " << MAGENTA << row[0] << "\n";
			cout << GREEN << "   X" << WHITE << ": " << MAGENTA << row[1] << "\n";
			cout << GREEN << "   Y" << WHITE << ": " << MAGENTA << row[2] << "\n";
			cout << GREEN << "   Level" << WHITE << ": " << MAGENTA << row[3] << "\n";
			cout << GREEN << "   Max HP" << WHITE << ": " << MAGENTA << row[4] << "\n";
			cout << GREEN << "   Current HP" << WHITE << ": " << MAGENTA << row[5] << "\n";
			cout << GREEN << "   Rupees" << WHITE << ": " << MAGENTA << row[6] << "\n";
			cout << GREEN << "   Arrows" << WHITE << ": " << MAGENTA << row[7] << "\n";
			cout << GREEN << "   Bombs" << WHITE << ": " << MAGENTA << row[8] << "\n";
			cout << GREEN << "   Glove Power" << WHITE << ": " << MAGENTA << row[9] << "\n";
			cout << GREEN << "   Sword Power" << WHITE << ": " << MAGENTA << row[10] << "\n";
			cout << GREEN << "   Sheild Power" << WHITE << ": " << MAGENTA << row[11] << "\n\n";
			
			cout << GREEN << "   1." << WHITE << " Edit Attribute\n";
			cout << GREEN << "   2." << WHITE << " Edit Appearance\n";
			cout << GREEN << "   3." << WHITE << " Change Level\n";
			cout << GREEN << "   4." << WHITE << " Warp Options\n";
			cout << GREEN << "   5." << WHITE << " Add to Guild\n";
			cout << GREEN << "   6." << WHITE << " Remove from Guild\n";
			cout << GREEN << "   0." << WHITE << " Return\n\n";
			cout << RED << "   Select Menu Number [0-8]" << WHITE << std::endl; 
			
			cout << WHITE; 
			
			clearConsole(ROWS - 27);
			cout << YELLOW << "Input: " << WHITE; 
			
			cin >> choice;
			
			switch (choice) {
				case 1: {
					menu = 5;
					choice = -1;
					break;
				}
				case 2: {
					menu = 4;
					choice = -1;
					break;
				}
				case 3:
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					updatePlayerLevel(conn, table_name, accname);
					break;
				case 4: {
					menu = 3;
					choice = -1;
					break;
				}
				case 5:
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					addGuildMember(conn, accname);
					break;
				case 6:
					removeGuildMember(conn, accname);
					break;
				case 0:
					menu = 0;
					choice = -1;
					break;
				}
			

		}
		
		if (menu == 3) {
			// Teleport Menu
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "Warp Menu\n" << endl; 
			cout << GREEN << "   1. " << WHITE << "Warp All Guild Members to " << nickname << "" << "\n";
			cout << GREEN << "   2. " << WHITE << "Warp Selected Players to " << nickname << "\n";
			cout << GREEN << "   0. " << WHITE << "Return\n\n";
			cout << RED << "   Select Menu Number [0-4]" << WHITE << std::endl; 
			clearConsole(ROWS - 9);
			cout << YELLOW << "Input: " << WHITE; 
			cin >> choice;
					
			switch (choice) {
				case 1:
					menu = 7;
					returnmenu = 3;
					break;
				case 2:
					menu = 6;
					returnmenu = 3;
					break;
				default:
					cout << "Invalid option selected." << endl;
					menu = 1;
					choice = -1;
					break;
			}	
			
		}
		
		if (menu == 4) {
			//Appearance Menu
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "User Appearance\n" << endl; 
					
			cout << GREEN << "   1. " << WHITE << "Head Image" << WHITE << ": " << MAGENTA << row[12] << "\n";
			cout << GREEN << "   2. " << WHITE << "Sword Image" << WHITE << ": " << MAGENTA << row[13] << "\n";
			cout << GREEN << "   3. " << WHITE << "Shield Image" << WHITE << ": " << MAGENTA << row[14] << "\n";
			cout << GREEN << "   0. " << WHITE << "Return\n\n";
			// Define the list of attributes
			vector<string> attributes = {"headimg", "swordimg", "shieldimg"};
			cout << RED << "   Select Attribute Number [0-3]" << WHITE << std::endl; 
			clearConsole(ROWS - 10);
			cout << YELLOW << "Input: " << WHITE; 
			cin >> choice;
			if (choice < 1 || choice > attributes.size()) {
				cout << "Returning..." << endl;
			} else {
				string attribute_name = attributes[choice-1];
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				updatePlayerProperty(conn, table_name, accname, attribute_name, "Enter the new image (press enter to keep current image): ");			
			}
			menu = 1;
			choice = -1;
		}
		
		if (menu == 5) {
		
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "User Attributes\n" << endl; 
					
			cout << GREEN << "    1. " << WHITE << "Account Name" << WHITE << ": " << MAGENTA << accname << "\n";
			cout << GREEN << "    2. " << WHITE << "Nick Name" << WHITE << ": " << MAGENTA << row[0] << "\n";
			cout << GREEN << "    3. " << WHITE << "X" << WHITE << ": " << MAGENTA << row[1] << "\n";
			cout << GREEN << "    4. " << WHITE << "Y" << WHITE << ": " << MAGENTA << row[2] << "\n";
			cout << GREEN << "    5. " << WHITE << "Level" << WHITE << ": " << MAGENTA << row[3] << "\n";
			cout << GREEN << "    6. " << WHITE << "Max HP" << WHITE << ": " << MAGENTA << row[4] << "\n";
			cout << GREEN << "    7. " << WHITE << "Current HP" << WHITE << ": " << MAGENTA << row[5] << "\n";
			cout << GREEN << "    8. " << WHITE << "Rupees" << WHITE << ": " << MAGENTA << row[6] << "\n";
			cout << GREEN << "    9. " << WHITE << "Arrows" << WHITE << ": " << MAGENTA << row[7] << "\n";
			cout << GREEN << "   10. " << WHITE << "Bombs" << WHITE << ": " << MAGENTA << row[8] << "\n";
			cout << GREEN << "   11. " << WHITE << "Glove Power" << WHITE << ": " << MAGENTA << row[9] << "\n";
			cout << GREEN << "   12. " << WHITE << "Sword Power" << WHITE << ": " << MAGENTA << row[10] << "\n";
			cout << GREEN << "   13. " << WHITE << "Sheild Power" << WHITE << ": " << MAGENTA << row[11] << "\n";
			cout << GREEN << "    0. " << WHITE << "Return\n\n";
		
			// Define the list of attributes
			vector<string> attributes = {"accname", "nickname", "x", "y", "level", "maxhp", "hp", "rupees", "arrows", "bombs", "glovepower", "swordpower", "shieldpower"};
			cout << RED << "   Select Attribute Number [0-13]" << WHITE << std::endl; 
			clearConsole(ROWS - 20);
			cout << YELLOW << "Input: " << WHITE; 
			cin >> choice;
			if (choice < 1 || choice > attributes.size()) {
				//cout << "Returning..." << endl;
			} else {
				string attribute_name = attributes[choice-1];
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				updatePlayerProperty(conn, table_name, accname, attribute_name, "Enter the new attribute (press enter to keep current attribute): ");	
			}
			choice = -1;
			menu = 1;
		}
		
		if (menu == 6) {
			// Teleport multiple players to a single player.

			table_name = getTableName();
			if (returnmenu == 2) {
				accname = getAccname(accselect);
			}

			//Grab the level to teleport to.
			vector<std::string> fields = {"nickname", "level", "x", "y"};
			vector<std::string> values;
			selectFieldsFromTable(conn, table_name, accname, fields, values);
			nickname = values[0];
			level = values[1];
			x = values[2];
			y = values[3];

			cout << nickname << " is at level " << level << " at coordinates " << x << ", " << y << "." << endl;

			// Teleport each player to the destination player
			while (true) {
				cout << "Enter user name (or type 'quit' to exit): ";
				getline(cin, accselect);
				if (accselect == "quit") {
					break;
				}

				//Grab the current level we are warping from.
				vector<std::string> fields = {"nickname", "level", "x", "y"};
				vector<std::string> values;
				selectFieldsFromTable(conn, table_name, accselect, fields, values);
				player = values[0];
				levelselect = values[1];
				xselect = values[2];
				yselect = values[3];
						
				string query = "UPDATE " + table_name + " SET level='" + level + "', x='" + x + "', y='" + y + "' WHERE accname='" + accselect + "'";
				ControlDBUpdates(conn, accselect, "level");
				ControlDBUpdates(conn, accselect, "x");
				ControlDBUpdates(conn, accselect, "y");
				// Execute the query
				if (mysql_query(conn, query.c_str())) {
					// If an error occurred, print the error message
					cerr << "Error: " << mysql_error(conn) << endl;
				}
				cout << "Player " << player << " is at level " << levelselect << " at coordinates " << xselect << ", " << yselect << ". Teleporting them to " << nickname << " who is at " << level << "." << endl;
			}
			menu = returnmenu;
			
		}
		
		if (menu == 7) {
			// Teleport multiple players to a single player.

			table_name = getTableName();
			if (returnmenu == 2) {
				accname = getAccname(accselect);
			}


					
			//Grab the level to teleport to.
			vector<std::string> fields = {"nickname", "level", "x", "y"};
			vector<std::string> values;
			selectFieldsFromTable(conn, table_name, accname, fields, values);
			nickname = values[0];
			level = values[1];
			x = values[2];
			y = values[3];
					
			guildname = getGuildplayer(guildnameselect);
					
			cout << nickname << " is at " << level << " at coordinates " << x << ", " << y << "." << endl;

			string query = "SELECT accname, player FROM guilds WHERE guildname='" + guildname + "'";
			if (mysql_query(conn, query.c_str())) {
				// handle error
			}

			MYSQL_RES* res = mysql_store_result(conn);
			if (mysql_num_rows(res) == 0) {
				cout << "No guild member with the specified account name found." << endl;
				break;
			}
								
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res))) {
				string accselect = row[0];
				string player = row[1];
				
				//Grab the current level we are warping from.
				vector<std::string> fields = {"nickname", "level", "x", "y"};
				vector<std::string> values;
				selectFieldsFromTable(conn, table_name, accselect, fields, values);
				playerselect = values[0];
				levelselect = values[1];
				xselect = values[2];
				yselect = values[3];
						
				string query = "UPDATE " + table_name + " SET level='" + level + "', x='" + x + "', y='" + y + "' WHERE accname='" + accselect + "'";

				ControlDBUpdates(conn, accselect, "level");
				ControlDBUpdates(conn, accselect, "x");
				ControlDBUpdates(conn, accselect, "y");

				// Execute the query
				if (mysql_query(conn, query.c_str())) {
					// If an error occurred, print the error message
					cerr << "Error: " << mysql_error(conn) << endl;
				}
						
				cout << "Player " << player << " is at level " << levelselect << " at coordinates " << xselect << ", " << yselect << ". Teleporting them to " << nickname << " who is at " << level << "." << endl;

			}
			menu = returnmenu;
			std::cout << "Press any key to continue...\n";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		if (menu == 2) {
			cout << CYAN << "\n" << branding << "\n\n" << WHITE << "Miscellaneous Items\n" << endl; 
			cout << GREEN << "   1." << WHITE << " List tables\n";
			cout << GREEN << "   2." << WHITE << " Show table contents\n";
			cout << GREEN << "   3." << WHITE << " Show fields of a table\n";
			cout << GREEN << "   4." << WHITE << " Add guild member\n";
			cout << GREEN << "   5." << WHITE << " Remove guild member\n";
			cout << GREEN << "   6." << WHITE << " Change player level\n";
			cout << GREEN << "   7." << WHITE << " Change player's name to guild name\n";
			cout << GREEN << "   8." << WHITE << " Teleport players to nominated player\n";
			cout << GREEN << "   9." << WHITE << " Teleport guild to nominated player\n";
			cout << GREEN << "   0." << WHITE << " Return\n\n";
			cout << RED << "   Select Menu Number [0-9]" << WHITE << std::endl; 
			const auto [ROWS,COLS] = get_terminal_size();
			clearConsole(ROWS - 16);
			cout << YELLOW << "Input: " << WHITE; 
			cin >> choice;
			switch (choice) {
				case 1: {
					// List tables
					mysql_query(conn, "SHOW TABLES");

					res = mysql_store_result(conn);
					while (row = mysql_fetch_row(res)) {
						cout << row[0] << endl;
					}

					
					cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input stream
					break;
				}
				case 2: {
					// Show table contents
					string table_name;
					cout << "Enter table name: ";
					cin >> table_name;
					mysql_query(conn, ("SELECT * FROM " + table_name).c_str());
					res = mysql_store_result(conn);
					while (row = mysql_fetch_row(res)) {
						for (unsigned int i = 0; i < mysql_num_fields(res); i++) {
							cout << row[i] << "\t";
						}
						cout << endl;
					}
					cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input stream
					break;
				}
				case 3: {
					// Show fields of a table
					string table_name;
					cout << "Enter table name: ";
					cin >> table_name;
					mysql_query(conn, ("SELECT * FROM " + table_name).c_str());
					res = mysql_store_result(conn);
					// Show field names
					while (field = mysql_fetch_field(res)) {
						cout << field->name << "\t";
					}
					cout << endl;
					cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input stream
					break;
				}
				case 4: {
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					accname = getAccname(accselect);
					addGuildMember(conn, accname);
					break;
				}
				case 5: {
					//string accname, player, guildname;
					// Remove Guild Member
					cout << "Enter the account name of the guild member to remove: ";
					cin >> accname;

					removeGuildMember(conn, accname);


					break;
				}
				case 6: {
					// Update player level
					table_name = getTableName();
					accname = getAccname(accselect);
					updatePlayerLevel(conn, table_name, accname);
					break;
				}
				case 7: {
					// Update player name based on guild
					table_name = getTableName();
					accname = getAccname(accselect);
					
					mysql_query(conn, ("SELECT guildname, player FROM guilds WHERE accname='" + accname + "'").c_str());
					res = mysql_store_result(conn);

					if (mysql_num_rows(res) == 0) {
						cout << "No accname with the specified name found." << endl;
						
						break;
					}
					while (row = mysql_fetch_row(res)) {
						guildname = row[0];
						player = row[1];
						mysql_query(conn, ("UPDATE " + table_name + " SET nickname='" + player + " (" + guildname + ")' WHERE accname='" + accname + "'").c_str());
						cout << "Updated player " << player << " in " << table_name << " table." << endl;
					}
					
					break;
				}
				case 8: {
					menu = 6;
					choice = -1;
					returnmenu = 2;
					break;
				}
					
				case 9: {
					menu = 7;
					choice = -1;
					returnmenu = 2;
					break;
				}
				case 0: {
					// Exit
					menu = 0;
					choice = -1;
					break;
				}
				default: {
					// Invalid choice
					cout << "Invalid choice." << endl;
					break;
				}
			}
			if (res != nullptr) {
				//Clear res, but only when it is used!
				mysql_free_result(res); 
			}
			if (returnmenu == 0) {
				std::cout << "Press any key to continue...\n";
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
		}
    }
    mysql_close(conn);
    return 0;
}