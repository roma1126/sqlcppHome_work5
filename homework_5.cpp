#include <iostream> 
#include <string>
#include <pqxx/pqxx> 

class Database
{
public:
	void createTables(pqxx::work& tx) 
	{
		tx.exec("CREATE TABLE IF NOT EXISTS client ("
			"client_id SERIAL PRIMARY KEY, "
			"first_name VARCHAR(80) NOT NULL, "
			"last_name VARCHAR(80) NOT NULL, "
			"email VARCHAR(80)); "
			"CREATE TABLE IF NOT EXISTS phone ("
			"phone_id SERIAL PRIMARY KEY, "
			"client_id INTEGER NOT NULL references client(client_id), "
			"phone VARCHAR(40));");
		tx.commit();
	}

	void addNewClient(pqxx::work& tx)
	{
		std::string firstName, lastName, email;
		std::cout << "Enter the first name, last name and email of the new client: " << std::endl;
		std::cin >> firstName >> lastName >> email;

		tx.exec_prepared("insert_client", firstName, lastName, email);
		tx.commit();
	}

	int getIdByName(pqxx::work& tx) 
	{
		std::string firstName, lastName;
		std::cout << "Enter the client's first and last name: " << std::endl;
		std::cin >> firstName >> lastName;

		auto count = tx.query_value <int>("SELECT COUNT(*) FROM client WHERE "
			"first_name = '" + tx.esc(firstName) + "' AND "
			"last_name = '" + tx.esc(lastName) + "'");

		if (count != 0) 
		{
			auto queryResult = tx.query <int>(
				"SELECT client_id FROM client WHERE "
				"first_name = '" + tx.esc(firstName) + "' AND "
				"last_name = '" + tx.esc(lastName) + "'");
			for (auto [id] : queryResult)
				return id;
		}
		else 
		{
			std::cout << "Tere is no client with this name in the database" << std::endl;
			return -1;
		}
	}

	void addPhoneNumber(pqxx::work& tx) 
	{
		std::string phoneNumber;
		int id = getIdByName(tx);
		if (id != -1) {
			std::cout << "Enter the phone number: " << std::endl;
			std::cin >> phoneNumber;

			tx.exec_prepared("insert_phone", id, phoneNumber);
			tx.commit();
		}
		else return;
	}

	void getDataById(pqxx::work& tx, int id) 
	{
		std::string sid = std::to_string(id);

		auto clientData = tx.query <std::string, std::string, std::string>(
			"SELECT first_name, last_name, email FROM client WHERE client_id = '" + sid + "';");
		system("cls");

		for (auto [fn, ln, em] : clientData) 
		{
			std::cout << fn << " " << ln << " " << em << std::endl;
		}
		auto clientPhones = tx.query <std::string>(
			"SELECT phone FROM phone WHERE client_id = '" + sid + "'");

		for (auto [ph] : clientPhones) 
		{
			std::cout << ph << std::endl;
		}
		std::cout << std::endl;
	}

	void changeClientData(pqxx::work& tx)
	{
		std::string newFirstName, newLastName, newEmail, newPhone;
		int id = getIdByName(tx);
		if (id != -1) {
			std::string sid = std::to_string(id);
			getDataById(tx, id);

			int k;
			std::cout << "What data do you want to change?" << std::endl;
			std::cout << "1) first name" << std::endl;
			std::cout << "2) last name" << std::endl;
			std::cout << "3) email" << std::endl;
			std::cout << "4) phone number" << std::endl;
			std::cin >> k;
			switch (k) 
			{

			case 1:
				std::cout << "Enter a new first name: ";
				std::cin >> newFirstName;
				tx.exec("UPDATE client SET first_name = ('" + tx.esc(newFirstName) + "') WHERE client_id = '" + sid + "'");
				tx.commit();
				break;
			case 2:
				std::cout << "Enter a new last name: ";
				std::cin >> newLastName;
				tx.exec("UPDATE client SET last_name = ('" + tx.esc(newLastName) + "') WHERE client_id = '" + sid + "'");
				tx.commit();
				break;
			case 3:
				std::cout << "Enter a new email: ";
				std::cin >> newEmail;
				tx.exec("UPDATE client SET email = ('" + tx.esc(newEmail) + "') WHERE client_id = '" + sid + "'");
				tx.commit();
				break;
			case 4:
				std::cout << "Enter a new phone: ";
				std::cin >> newPhone;
				tx.exec("UPDATE phone SET phone = ('" + tx.esc(newPhone) + "') WHERE client_id = '" + sid + "'");
				tx.commit();
				break;
			default:
				std::cout << "invalid input " << std::endl;
				break;
			}
		}
		else return;
	}

	void deleteClientPhone(pqxx::work& tx) 
	{
		std::string id = std::to_string(getIdByName(tx));
		tx.exec("DELETE FROM phone WHERE client_id = '" + id + "'");
		tx.commit();
		system("cls");
	}

	void deleteClient(pqxx::work& tx)
	{
		std::string id = std::to_string(getIdByName(tx));
		tx.exec("DELETE FROM phone WHERE client_id = '" + id + "'");
		tx.exec("DELETE FROM client WHERE client_id = '" + id + "'");
		tx.commit();
		system("cls");
	}

	void searchClient(pqxx::work& tx) 
	{
		std::string value;
		std::cout << "Enter the client's first name, last name, email or phone: " << std::endl;
		std::cin >> value;
		auto count = tx.query_value <int>("SELECT COUNT(*) FROM client WHERE "
			"first_name = '" + tx.esc(value) + "' OR "
			"last_name = '" + tx.esc(value) + "' OR "
			"email = '" + tx.esc(value) + "' ");

		if (count != 0) 
		{
			auto queryId = tx.query <int>(
				"SELECT client_id FROM client WHERE "
				"first_name = '" + tx.esc(value) + "' OR "
				"last_name = '" + tx.esc(value) + "' OR "
				"email = '" + tx.esc(value) + "' ");

			for (auto [id] : queryId) 
			{
				getDataById(tx, id);
			}
		}
		else 
		{
			auto count2 = tx.query_value <int>(
				"SELECT COUNT(*) FROM phone WHERE phone = '" + tx.esc(value) + "' ");
			if (count2 != 0)
			{
				auto queryId = tx.query <int>(
					"SELECT client_id FROM phone WHERE "
					"phone = '" + tx.esc(value) + "' ");
				for (auto [id] : queryId)
					getDataById(tx, id);
			}
			else 
			{
				std::cout << "Tere is no client with this name in the database" << std::endl;
				system("pause");
			}
		}
	}

};


int main()
{
	setlocale(LC_ALL, "Rus");

	try 
	{
		Database db;

		pqxx::connection c(
			"host=localhost "
			"port=5432 "
			"dbname=postgres "
			"user=postgres "
			"password=Bltjkjubz_39 ");

		pqxx::work tx0{ c };
		db.createTables(tx0);
		c.prepare("insert_client", "INSERT INTO client(first_name, last_name, email) VALUES ($1, $2, $3)");
		c.prepare("insert_phone", "INSERT INTO phone(client_id, phone) VALUES ($1, $2)");

		int key;
		while (true) {
			pqxx::work tx{ c };
			std::cout << std::endl;
			std::cout << "Select an action to work with the database: " << std::endl;
			std::cout << "1 - add a new client" << std::endl;
			std::cout << "2 - add a phone number" << std::endl;
			std::cout << "3 - change data" << std::endl;
			std::cout << "4 - delete phone" << std::endl;
			std::cout << "5 - delete client" << std::endl;
			std::cout << "6 - search client" << std::endl;
			std::cout << "7 - exit" << std::endl;


			std::cin >> key;
			system("cls");
			switch (key) {
			case 1:
				db.addNewClient(tx);
				continue;
			case 2:
				db.addPhoneNumber(tx);
				continue;
			case 3:
				db.changeClientData(tx);
				continue;
			case 4:
				db.deleteClientPhone(tx);
				continue;
			case 5:
				db.deleteClient(tx);
				continue;
			case 6:
				db.searchClient(tx);
				break;
			case 7:
				return 0;
			default:
				std::cout << "invalid input " << std::endl;
				break;
			}
		}
	}
	catch (pqxx::sql_error e) {
		std::cout << "Error happened: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Unknown error happened" << std::endl;
	}
	return 0;
}