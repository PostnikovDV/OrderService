#pragma once

class DatabaseManager
{
private:
    std::string m_connectionString;

public:
    DatabaseManager(const std::string& conn_str) : m_connectionString(conn_str) {}
    const std::string& GetConnectionString() { return m_connectionString; }
    bool CreateOrdersTable()
    {
        try
        {
            pqxx::connection conn(m_connectionString);
            pqxx::work txn(conn);

            const char* create_table_sql = R"(
                CREATE TABLE IF NOT EXISTS orders (
                    order_id BIGSERIAL PRIMARY KEY,
                    product_id BIGINT NOT NULL,
                    amount INTEGER NOT NULL,
                    email_client VARCHAR(255) NOT NULL,
                    price NUMERIC(15,2) NOT NULL,
                    phone_number VARCHAR(20) NOT NULL
                )
            )";

            txn.exec(create_table_sql);
            txn.commit();

            std::cout << "Orders table created successfully" << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Database error: " << e.what() << std::endl;
            return false;
        }
    }

    bool TestConnection()
    {
        try
        {
            pqxx::connection conn(m_connectionString);
            if (conn.is_open())
            {
                std::cout << "Database connection: OK" << std::endl;
                return true;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Database connection failed: " << e.what() << std::endl;
            return false;
        }
    }
};