////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or utilied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#include "sync/remote_mongo_collection.hpp"

namespace realm {
namespace app {

static void handle_response(util::Optional<AppError> error,
                            util::Optional<std::string> value,
                            std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    if (value && !error) {
        return completion_block(value.value(), error);
    }
    
    return completion_block("", error);
}

static void handle_count_response(util::Optional<AppError> error,
                                  util::Optional<std::string> value,
                                  std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    if (value && !error) {
        try {
            auto json = nlohmann::json::parse(value.value());
            auto count_string = json.at("$numberLong").get<std::string>();
            return completion_block(std::stoi(count_string), error);
        } catch (const std::exception& e) {
            return completion_block(0, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
        }
    }
    
    return completion_block(0, error);
}

static void handle_delete_count_response(util::Optional<AppError> error,
                                         util::Optional<std::string> value,
                                         std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    if (value && !error) {
        try {
            auto json = nlohmann::json::parse(value.value());
            auto count_string = json.at("deletedCount").at("$numberInt").get<std::string>();
            return completion_block(std::stoi(count_string), error);
        } catch (const std::exception& e) {
            return completion_block(0, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
        }
    }
    
    return completion_block(0, error);
}

static void handle_update_response(util::Optional<AppError> error,
                                   util::Optional<std::string> value,
                                   std::function<void(RemoteMongoCollection::RemoteUpdateResult, util::Optional<AppError>)> completion_block)
{
    if (value && !error) {
        
        try {
            auto json = nlohmann::json::parse(value.value());
            auto matched_count_string = json.at("matchedCount").at("$numberInt").get<std::string>();
            auto matched_count = std::stoull(matched_count_string);
            auto modified_count_string = json.at("modifiedCount").at("$numberInt").get<std::string>();
            auto modified_count = std::stoull(modified_count_string);
            std::string upserted_id;
            auto it = json.find("upsertedId");
            if (it != json.end()) {
                upserted_id = json.at("upsertedId").at("$oid").get<std::string>();
            }
            
            return completion_block(RemoteMongoCollection::RemoteUpdateResult {
                matched_count,
                modified_count,
                upserted_id
            }, error);
        } catch (const std::exception& e) {
            return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
        }
    }
    
    return completion_block({}, error);
}

void RemoteMongoCollection::find(const std::string& filter_json,
                                 RemoteFindOptions options,
                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
        
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        if (options.limit) {
            args.push_back({ "limit", options.limit.value() });
        }
        
        if (options.projection_json) {
            args.push_back({ "project", nlohmann::json::parse(options.projection_json.value()) });
        }
        
        if (options.projection_json) {
            args.push_back({ "sort", nlohmann::json::parse(options.sort_json.value()) });
        }
                
        m_service->call_function("find",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::find(const std::string& filter_json,
                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    find(filter_json, {}, completion_block);
}

void RemoteMongoCollection::find_one(const std::string& filter_json,
                                 RemoteFindOptions options,
                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
        
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        if (options.limit) {
            args.push_back({ "limit", options.limit.value() });
        }
        
        if (options.projection_json) {
            args.push_back({ "project", nlohmann::json::parse(options.projection_json.value()) });
        }
        
        if (options.projection_json) {
            args.push_back({ "sort", nlohmann::json::parse(options.sort_json.value()) });
        }
                
        m_service->call_function("findOne",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::find_one(const std::string& filter_json,
                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    find_one(filter_json, {}, completion_block);
}

void RemoteMongoCollection::insert_one(const std::string& value_json,
                                       std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "document", nlohmann::json::parse(value_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
            
        m_service->call_function("insertOne",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
       return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::aggregate(std::vector<std::string> pipline,
                                      std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    try {
        auto pipelines = nlohmann::json::array();
        for (std::string& pipeline_json : pipline) {
            pipelines.push_back(nlohmann::json::parse(pipeline_json));
        }
        
        auto base_args = m_base_operation_args;
        base_args.push_back({ "pipeline", pipelines });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
         
        m_service->call_function("aggregate",
                              args.dump(),
                              [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::count(const std::string& filter_json,
                                  uint64_t limit,
                                  std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        base_args.push_back({ "limit", limit });
        
        m_service->call_function("count",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_count_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block(0, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::count(const std::string& filter_json,
                                  std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    count(filter_json, {}, completion_block);
}

void RemoteMongoCollection::insert_many(std::vector<std::string> documents,
                                        std::function<void(std::map<uint64_t, std::string>, util::Optional<AppError>)> completion_block)
{
     try {

         auto documents_array = nlohmann::json::array();
         for (std::string& document_json : documents) {
             documents_array.push_back(nlohmann::json::parse(document_json));
         }

         auto base_args = m_base_operation_args;
         base_args.push_back({ "documents", documents_array });
         auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );

         m_service->call_function("insertMany",
                                  args.dump(),
                                  [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
             if (value && !error) {
                 try {
                     auto json = nlohmann::json::parse(value.value());
                     auto inserted_ids = json.at("insertedIds").get<std::vector<nlohmann::json>>();
                     auto mapped_oids = std::map<uint64_t, std::string>();
                     int i = 0;
                     for(auto& inserted_id : inserted_ids) {
                         auto oid = inserted_id.at("$oid").get<std::string>();
                         mapped_oids.insert({i, oid});
                         i++;
                     }
                     return completion_block(mapped_oids, error);
                 } catch (const std::exception& e) {
                     return completion_block(std::map<uint64_t, std::string>(), AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
                 }
             }

             return completion_block(std::map<uint64_t, std::string>(), error);
         });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::delete_one(const std::string& filter_json,
                                       std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    try {

         auto base_args = m_base_operation_args;
         base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
         auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );

         m_service->call_function("deleteOne",
                                  args.dump(),
                                  [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
             handle_delete_count_response(error, value, completion_block);
         });
    } catch (const std::exception& e) {
        return completion_block(0, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::delete_many(const std::string& filter_json,
                                        std::function<void(uint64_t, util::Optional<AppError>)> completion_block)
{
    try {

         auto base_args = m_base_operation_args;
         base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
         auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );

         m_service->call_function("deleteMany",
                                  args.dump(),
                                  [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
             handle_delete_count_response(error, value, completion_block);
         });
    } catch (const std::exception& e) {
        return completion_block(0, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::update_one(const std::string& filter_json,
                                       const std::string& update_json,
                                       bool upsert,
                                       std::function<void(RemoteMongoCollection::RemoteUpdateResult, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        base_args.push_back({ "update", nlohmann::json::parse(update_json) });
        base_args.push_back({ "upsert", upsert });
        auto args = nlohmann::json({{"arguments", nlohmann::json::array({base_args})}});

        m_service->call_function("updateOne",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_update_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::update_one(const std::string& filter_json,
                                       const std::string& update_json,
                                       std::function<void(RemoteMongoCollection::RemoteUpdateResult, util::Optional<AppError>)> completion_block)
{
    update_one(filter_json, update_json, {}, completion_block);
}

void RemoteMongoCollection::update_many(const std::string& filter_json,
                                        const std::string& update_json,
                                        bool upsert,
                                        std::function<void(RemoteMongoCollection::RemoteUpdateResult, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        base_args.push_back({ "update", nlohmann::json::parse(update_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );

        args.push_back({ "upsert", upsert });
                
        m_service->call_function("updateMany",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_update_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::update_many(const std::string& filter_json,
                                        const std::string& update_json,
                                        std::function<void(RemoteMongoCollection::RemoteUpdateResult, util::Optional<AppError>)> completion_block)
{
    update_many(filter_json, update_json, {}, completion_block);
}

void RemoteMongoCollection::find_one_and_update(const std::string& filter_json,
                                                const std::string& update_json,
                                                RemoteMongoCollection::RemoteFindOneAndModifyOptions options,
                                                std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        base_args.push_back({ "update", nlohmann::json::parse(update_json) });
        //auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        if (options.upsert) {
            base_args.push_back({ "upsert", options.upsert });
        }
        
        if (options.return_new_document) {
            base_args.push_back({ "returnNewDocument", options.return_new_document });
        }
        
        if (options.projection_json) {
            base_args.push_back({ "project", nlohmann::json::parse(options.projection_json.value()) });
        }
        
        if (options.projection_json) {
            base_args.push_back({ "sort", nlohmann::json::parse(options.sort_json.value()) });
        }
        
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );

        auto x = args.dump();
        m_service->call_function("findOneAndUpdate",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::find_one_and_update(const std::string& filter_json,
                                                const std::string& update_json,
                                                std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    find_one_and_update(filter_json, update_json, {}, completion_block);
}

void RemoteMongoCollection::find_one_and_replace(const std::string& filter_json,
                                                 const std::string& replacement_json,
                                                 RemoteMongoCollection::RemoteFindOneAndModifyOptions options,
                                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        base_args.push_back({ "update", nlohmann::json::parse(replacement_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        if (options.upsert) {
            args.push_back({ "upsert", options.upsert });
        }
        
        if (options.return_new_document) {
            args.push_back({ "returnNewDocument", options.return_new_document });
        }
        
        if (options.projection_json) {
            args.push_back({ "project", nlohmann::json::parse(options.projection_json.value()) });
        }
        
        if (options.projection_json) {
            args.push_back({ "sort", nlohmann::json::parse(options.sort_json.value()) });
        }
                
        m_service->call_function("findOneAndReplace",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string> value) {
            handle_response(error, value, completion_block);
        });
    } catch (const std::exception& e) {
        return completion_block({}, AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::find_one_and_replace(const std::string& filter_json,
                                                 const std::string& replacement_json,
                                                 std::function<void(std::string, util::Optional<AppError>)> completion_block)
{
    find_one_and_replace(filter_json, replacement_json, {}, completion_block);
}

void RemoteMongoCollection::find_one_and_delete(const std::string& filter_json,
                                                RemoteMongoCollection::RemoteFindOneAndModifyOptions options,
                                                std::function<void(util::Optional<AppError>)> completion_block)
{
    try {
        auto base_args = m_base_operation_args;
        base_args.push_back({ "query", nlohmann::json::parse(filter_json) });
        auto args = nlohmann::json( {{"arguments", nlohmann::json::array({base_args} ) }} );
        
        if (options.upsert) {
            args.push_back({ "upsert", options.upsert });
        }
        
        if (options.return_new_document) {
            args.push_back({ "returnNewDocument", options.return_new_document });
        }
        
        if (options.projection_json) {
            args.push_back({ "project", nlohmann::json::parse(options.projection_json.value()) });
        }
        
        if (options.projection_json) {
            args.push_back({ "sort", nlohmann::json::parse(options.sort_json.value()) });
        }

        m_service->call_function("findOneAndDelete",
                                 args.dump(),
                                 [completion_block](util::Optional<AppError> error, util::Optional<std::string>) {
            completion_block(error);
        });
    } catch (const std::exception& e) {
        return completion_block(AppError(make_error_code(JSONErrorCode::malformed_json), e.what()));
    }
}

void RemoteMongoCollection::find_one_and_delete(const std::string& filter_json,
                                                std::function<void(util::Optional<AppError>)> completion_block)
{
    find_one_and_delete(filter_json, {}, completion_block);
}

} // namespace app
} // namespace realm

