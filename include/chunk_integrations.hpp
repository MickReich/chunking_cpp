/**
 * @file chunk_integrations.hpp
 * @brief Integration utilities for external systems and databases
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include "chunk_common.hpp"
#include <memory>
#include <string>
#include <vector>

namespace chunk_integrations {

/**
 * @brief Abstract base class for database connections
 */
class CHUNK_EXPORT DatabaseConnection {
public:
    virtual ~DatabaseConnection() = default;

    /**
     * @brief Connect to database
     * @param connection_string Database connection parameters
     * @throws std::runtime_error if connection fails
     */
    virtual void connect(const std::string& connection_string) = 0;

    /**
     * @brief Disconnect from database
     */
    virtual void disconnect() = 0;
};

/**
 * @brief Class for storing chunks in various databases
 */
class CHUNK_EXPORT DatabaseChunkStore {
public:
    /**
     * @brief Constructor
     * @param connection Database connection instance
     * @param table_name Target table name
     */
    DatabaseChunkStore(std::unique_ptr<DatabaseConnection> connection,
                       const std::string& table_name);

    /**
     * @brief Store chunks in PostgreSQL database
     * @tparam T Data type of chunk elements
     * @param chunks Vector of chunks to store
     * @throws std::runtime_error if storage fails
     */
    template <typename T>
    void store_chunks_postgres(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Store chunks in MongoDB database
     * @tparam T Data type of chunk elements
     * @param chunks Vector of chunks to store
     * @throws std::runtime_error if storage fails
     */
    template <typename T>
    void store_chunks_mongodb(const std::vector<std::vector<T>>& chunks);

private:
    std::unique_ptr<DatabaseConnection> db_connection; ///< Database connection
    std::string table;                                 ///< Target table name
};

/**
 * @brief Abstract base class for message queue connections
 */
class CHUNK_EXPORT MessageQueueConnection {
public:
    virtual ~MessageQueueConnection() = default;

    /**
     * @brief Connect to message queue
     * @param connection_string Connection parameters
     */
    virtual void connect(const std::string& connection_string) = 0;

    /**
     * @brief Disconnect from message queue
     */
    virtual void disconnect() = 0;
};

/**
 * @brief Class for publishing chunks to message queues
 */
class CHUNK_EXPORT ChunkMessageQueue {
public:
    /**
     * @brief Constructor
     * @param connection Message queue connection instance
     * @param queue_name Target queue name
     */
    ChunkMessageQueue(std::unique_ptr<MessageQueueConnection> connection,
                      const std::string& queue_name);

    /**
     * @brief Publish chunks to Kafka topic
     * @tparam T Data type of chunk elements
     * @param chunks Vector of chunks to publish
     * @throws std::runtime_error if publishing fails
     */
    template <typename T>
    void publish_chunks_kafka(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Publish chunks to RabbitMQ queue
     * @tparam T Data type of chunk elements
     * @param chunks Vector of chunks to publish
     * @throws std::runtime_error if publishing fails
     */
    template <typename T>
    void publish_chunks_rabbitmq(const std::vector<std::vector<T>>& chunks);

private:
    std::unique_ptr<MessageQueueConnection> mq_connection; ///< Message queue connection
    std::string queue;                                     ///< Target queue name
};

} // namespace chunk_integrations