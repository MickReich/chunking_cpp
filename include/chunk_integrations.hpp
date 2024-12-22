#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

// Optional dependencies
#ifdef HAVE_POSTGRESQL
#include <pqxx/pqxx>          // PostgreSQL
#endif

#ifdef HAVE_MONGODB
#include <mongocxx/client.hpp> // MongoDB
#endif

#ifdef HAVE_KAFKA
#include <cppkafka/cppkafka.h>// Kafka
#endif

#ifdef HAVE_RABBITMQ
#include <amqp.h>             // RabbitMQ
#endif

#ifdef HAVE_JSON
#include <nlohmann/json.hpp>  // JSON serialization
#endif

namespace chunk_integrations {

#ifdef HAVE_JSON
using json = nlohmann::json;
#else
// Fallback JSON implementation if nlohmann::json is not available
struct json {
    template<typename T>
    static std::string dump(const T&) { 
        throw std::runtime_error("JSON support not enabled"); 
    }
};
#endif

/**
 * @brief Base class for database connections
 */
class DatabaseConnection {
public:
    virtual ~DatabaseConnection() = default;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};

/**
 * @brief PostgreSQL connection handler
 */
#ifdef HAVE_POSTGRESQL
class PostgresConnection : public DatabaseConnection {
private:
    std::unique_ptr<pqxx::connection> conn;
    std::string connection_string;

public:
    explicit PostgresConnection(const std::string& conn_string)
        : connection_string(conn_string) {}

    bool connect() override {
        try {
            conn = std::make_unique<pqxx::connection>(connection_string);
            return conn->is_open();
        } catch (const std::exception& e) {
            std::cerr << "PostgreSQL connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() override {
        if (conn) {
            conn->disconnect();
        }
    }

    bool is_connected() const override {
        return conn && conn->is_open();
    }

    pqxx::connection* get_connection() {
        return conn.get();
    }
};
#endif

/**
 * @brief MongoDB connection handler
 */
#ifdef HAVE_MONGODB
class MongoConnection : public DatabaseConnection {
private:
    mongocxx::client client;
    std::string uri;
    bool connected{false};

public:
    explicit MongoConnection(const std::string& mongodb_uri)
        : uri(mongodb_uri) {}

    bool connect() override {
        try {
            mongocxx::uri uri(this->uri);
            client = mongocxx::client(uri);
            connected = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "MongoDB connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() override {
        connected = false;
    }

    bool is_connected() const override {
        return connected;
    }

    mongocxx::client* get_client() {
        return &client;
    }
};
#endif

/**
 * @brief Database chunk store implementation
 */
class DatabaseChunkStore {
private:
    std::unique_ptr<DatabaseConnection> connection;
    std::string table_name;
    
    // Helper method to convert chunk to JSON
    template<typename T>
    json chunk_to_json(const std::vector<T>& chunk, size_t chunk_id) {
        return {
            {"chunk_id", chunk_id},
            {"size", chunk.size()},
            {"data", chunk},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };
    }

public:
    DatabaseChunkStore(std::unique_ptr<DatabaseConnection> conn, 
                      const std::string& table = "chunks")
        : connection(std::move(conn))
        , table_name(table) {}

    template<typename T>
    void store_chunks_postgres(const std::vector<std::vector<T>>& chunks) {
        if (!connection->is_connected()) {
            throw std::runtime_error("Database not connected");
        }

        auto* postgres = dynamic_cast<PostgresConnection*>(connection.get());
        if (!postgres) {
            throw std::runtime_error("Invalid connection type");
        }

        try {
            pqxx::work txn(*postgres->get_connection());
            
            for (size_t i = 0; i < chunks.size(); ++i) {
                json chunk_json = chunk_to_json(chunks[i], i);
                
                txn.exec_params(
                    "INSERT INTO " + table_name + " (chunk_id, data, metadata) "
                    "VALUES ($1, $2, $3)",
                    i,
                    chunk_json["data"].dump(),
                    chunk_json.dump()
                );
            }
            
            txn.commit();
        } catch (const std::exception& e) {
            throw std::runtime_error("PostgreSQL error: " + std::string(e.what()));
        }
    }

    template<typename T>
    void store_chunks_mongodb(const std::vector<std::vector<T>>& chunks) {
        if (!connection->is_connected()) {
            throw std::runtime_error("Database not connected");
        }

        auto* mongo = dynamic_cast<MongoConnection*>(connection.get());
        if (!mongo) {
            throw std::runtime_error("Invalid connection type");
        }

        try {
            auto collection = mongo->get_client()->database("chunks")[table_name];
            
            std::vector<bsoncxx::document::value> documents;
            for (size_t i = 0; i < chunks.size(); ++i) {
                json chunk_json = chunk_to_json(chunks[i], i);
                documents.push_back(
                    bsoncxx::from_json(chunk_json.dump())
                );
            }
            
            collection.insert_many(documents);
        } catch (const std::exception& e) {
            throw std::runtime_error("MongoDB error: " + std::string(e.what()));
        }
    }
};

/**
 * @brief Base class for message queue connections
 */
class MessageQueueConnection {
public:
    virtual ~MessageQueueConnection() = default;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};

/**
 * @brief Kafka connection handler
 */
class KafkaConnection : public MessageQueueConnection {
private:
    std::unique_ptr<cppkafka::Producer> producer;
    std::string brokers;
    bool connected{false};

public:
    explicit KafkaConnection(const std::string& kafka_brokers)
        : brokers(kafka_brokers) {}

    bool connect() override {
        try {
            cppkafka::Configuration config = {
                {"metadata.broker.list", brokers}
            };
            producer = std::make_unique<cppkafka::Producer>(config);
            connected = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Kafka connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() override {
        producer.reset();
        connected = false;
    }

    bool is_connected() const override {
        return connected;
    }

    cppkafka::Producer* get_producer() {
        return producer.get();
    }
};

/**
 * @brief RabbitMQ connection handler
 */
class RabbitMQConnection : public MessageQueueConnection {
private:
    amqp_connection_state_t conn{nullptr};
    std::string uri;
    bool connected{false};

public:
    explicit RabbitMQConnection(const std::string& rabbitmq_uri)
        : uri(rabbitmq_uri) {}

    bool connect() override {
        try {
            conn = amqp_new_connection();
            amqp_connection_info info;
            amqp_parse_url(uri.c_str(), &info);
            
            auto socket = amqp_tcp_socket_new(conn);
            amqp_socket_open(socket, info.host, info.port);
            amqp_login(conn, info.vhost, 0, 131072, 0, 
                      AMQP_SASL_METHOD_PLAIN, info.user, info.password);
            
            connected = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "RabbitMQ connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() override {
        if (conn) {
            amqp_connection_close(conn);
            amqp_destroy_connection(conn);
            conn = nullptr;
        }
        connected = false;
    }

    bool is_connected() const override {
        return connected;
    }

    amqp_connection_state_t get_connection() {
        return conn;
    }
};

/**
 * @brief Message queue chunk publisher
 */
class ChunkMessageQueue {
private:
    std::unique_ptr<MessageQueueConnection> connection;
    std::string topic;
    
    template<typename T>
    std::string serialize_chunk(const std::vector<T>& chunk, size_t chunk_id) {
        json chunk_json = {
            {"chunk_id", chunk_id},
            {"size", chunk.size()},
            {"data", chunk},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };
        return chunk_json.dump();
    }

public:
    ChunkMessageQueue(std::unique_ptr<MessageQueueConnection> conn,
                     const std::string& topic_name)
        : connection(std::move(conn))
        , topic(topic_name) {}

    template<typename T>
    void publish_chunks_kafka(const std::vector<std::vector<T>>& chunks) {
        if (!connection->is_connected()) {
            throw std::runtime_error("Message queue not connected");
        }

        auto* kafka = dynamic_cast<KafkaConnection*>(connection.get());
        if (!kafka) {
            throw std::runtime_error("Invalid connection type");
        }

        try {
            for (size_t i = 0; i < chunks.size(); ++i) {
                std::string message = serialize_chunk(chunks[i], i);
                
                kafka->get_producer()->produce(
                    cppkafka::MessageBuilder(topic)
                    .payload(message)
                    .key(std::to_string(i))
                );
            }
            
            kafka->get_producer()->flush();
        } catch (const std::exception& e) {
            throw std::runtime_error("Kafka error: " + std::string(e.what()));
        }
    }

    template<typename T>
    void publish_chunks_rabbitmq(const std::vector<std::vector<T>>& chunks) {
        if (!connection->is_connected()) {
            throw std::runtime_error("Message queue not connected");
        }

        auto* rabbitmq = dynamic_cast<RabbitMQConnection*>(connection.get());
        if (!rabbitmq) {
            throw std::runtime_error("Invalid connection type");
        }

        try {
            amqp_channel_open(rabbitmq->get_connection(), 1);
            
            for (size_t i = 0; i < chunks.size(); ++i) {
                std::string message = serialize_chunk(chunks[i], i);
                
                amqp_basic_publish(rabbitmq->get_connection(),
                                 1,
                                 amqp_cstring_bytes(""),
                                 amqp_cstring_bytes(topic.c_str()),
                                 0,
                                 0,
                                 nullptr,
                                 amqp_cstring_bytes(message.c_str()));
            }
            
            amqp_channel_close(rabbitmq->get_connection(), 1, AMQP_REPLY_SUCCESS);
        } catch (const std::exception& e) {
            throw std::runtime_error("RabbitMQ error: " + std::string(e.what()));
        }
    }
};

} // namespace chunk_integrations