#pragma once

class DatabaseManager
{
private:
    std::string m_connectionString;

public:
    DatabaseManager(const std::string& conn_str) : m_connectionString(conn_str) {}
    const std::string& GetConnectionString() { return m_connectionString; }
  
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