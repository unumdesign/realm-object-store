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

#ifndef REMOTE_MONGO_COLLECTION_HPP
#define REMOTE_MONGO_COLLECTION_HPP

#include "remote_mongo_read_operation.hpp"
#include "remote_mongo_result_types.hpp"
#include <realm/util/optional.hpp>
#include <json.hpp>
#include <string>
#include <vector>

namespace realm {
namespace app {

class RemoteMongoCollection {
    
public:
    
    /// The name of this collection.
    const std::string name;
    
    /// The name of the database containing this collection.
    const std::string database_name;

    RemoteMongoCollection(std::string name, std::string database_name) : name(name), database_name(database_name) { }
    
    /// Finds the documents in this collection which match the provided filter.
    /// @param filter_json A `Document` as a json string that should match the query.
    /// @param options Optional `RemoteFindOptions` to use when executing the command.
    /// @returns A json string of the resulting documents.
    std::string find(const std::string& filter_json, util::Optional<RemoteFindOptions> options);

    /// Returns one document as a json string from a collection or view which matches the
    /// provided filter. If multiple documents satisfy the query, this method
    /// returns the first document according to the query's sort order or natural
    /// order.
    /// @param filter_json A `Document` in the form of a json string that should match the query.
    /// @param options Optional `RemoteFindOptions` to use when executing the command.
    /// @returns The resulting json string or nil if no such document exists
    util::Optional<std::string> find_one(const std::string& filter_json,
                                         util::Optional<RemoteFindOptions> options);
    
    /// Runs an aggregation framework pipeline against this collection.
    /// @param pipline A `Document` array made up of jsons strings containing the pipeline of aggregation operations to perform.
    /// @returns A json string of the resulting documents.
    std::string aggregate(std::vector<std::string> pipline);

    /// Counts the number of documents in this collection matching the provided filter.
    /// @param filter_json A `Document` as a json string, the filter that documents must match in order to be counted.
    /// @param options Optional `RemoteCountOptions` to use when executing the command.
    /// @returns The count of the documents that matched the filter.
    int count(std::string filter_json, util::Optional<RemoteFindOptions> options);
    
    /// Encodes the provided value to BSON and inserts it. If the value is missing an identifier, one will be
    /// generated for it.
    /// @param value  A `json` value to encode and insert.
    /// @Returns The result of attempting to perform the insert.
    RemoteInsertOneResult insert_one(std::string value);
    
    /// Encodes the provided values to BSON and inserts them. If any values are missing identifiers,
    /// they will be generated.
    /// @param documents  The `json` values in a vector to insert.
    /// @Returns The result of attempting to perform the insert.
    RemoteInsertManyResult insert_many(std::vector<std::string> documents);
    
    /// Deletes a single matching document from the collection.
    /// @param filter_json A `Document` as a json string representing the match criteria.
    /// @Returns The result of performing the deletion.
    RemoteDeleteResult delete_one(const std::string& filter_json);

    /// Deletes multiple documents
    /// @param filter_json Document representing the match criteria
    /// @Returns The result of performing the deletion.
    RemoteDeleteResult delete_many(const std::string& filter_json);
    
    /// Updates a single document matching the provided filter in this collection.
    /// @param filter_json  A `Document` as a json string representing the match criteria.
    /// @param update_json  A `Document` as a json string representing the update to be applied to a matching document.
    /// @param options Optional `RemoteUpdateOptions` to use when executing the command.
    /// @Returns The result of attempting to update a document.
    RemoteUpdateResult update_one(const std::string& filter_json,
                                  const std::string& update_json,
                                  util::Optional<RemoteFindOptions> options);

    /// Updates multiple documents matching the provided filter in this collection.
    /// @param filter_json  A `Document` as a json string representing the match criteria.
    /// @param update_json  A `Document` as a json string representing the update to be applied to matching documents.
    /// @param options  Optional `RemoteUpdateOptions` to use when executing the command.
    /// @Returns The result of attempting to update multiple documents.
    RemoteUpdateResult update_many(const std::string& filter_json,
                                   const std::string& update_json,
                                   util::Optional<RemoteFindOptions> options);

    /// Updates a single document in a collection based on a query filter and
    /// returns the document in either its pre-update or post-update form. Unlike
    /// `update_one`, this action allows you to atomically find, update, and
    /// return a document with the same command. This avoids the risk of other
    /// update operations changing the document between separate find and update
    /// operations.
    ///
    /// @param filter_json  A `Document` as a json string that should match the query.
    /// @param update_json  A `Document` as a json string describing the update.
    /// @param options  Optional `RemoteFindOneAndModifyOptions` to use when executing the command.
    /// @Returns The resulting `Document` or nil if no such document exists
    util::Optional<std::string> find_one_and_update(const std::string& filter_json,
                                                    const std::string& update_json,
                                                    util::Optional<RemoteFindOneAndModifyOptions> options);
    
    /// Overwrites a single document in a collection based on a query filter and
    /// returns the document in either its pre-replacement or post-replacement
    /// form. Unlike `update_one`, this action allows you to atomically find,
    /// replace, and return a document with the same command. This avoids the
    /// risk of other update operations changing the document between separate
    /// find and update operations.
    /// @param filter_json  A `Document` that should match the query.
    /// @param replacement_json  A `Document` describing the update.
    /// @param options  Optional `RemoteFindOneAndModifyOptions` to use when executing the command.
    /// @Returns The resulting `Document` or nil if no such document exists
    util::Optional<std::string> find_one_and_replace(const std::string& filter_json,
                                                     const std::string& replacement_json,
                                                     util::Optional<RemoteFindOneAndModifyOptions> options);

    /// Removes a single document from a collection based on a query filter and
    /// returns a document with the same form as the document immediately before
    /// it was deleted. Unlike `delete_one`, this action allows you to atomically
    /// find and delete a document with the same command. This avoids the risk of
    /// other update operations changing the document between separate find and
    /// delete operations.
    /// @param filter  A `Document` as a json string that should match the query.
    /// @param options  Optional `RemoteFindOneAndModifyOptions` to use when executing the command.
    /// @Returns The resulting `Document` as a string or nil if no such document exists
    util::Optional<std::string> find_one_and_delete(const std::string& filter_json,
                                                       util::Optional<RemoteFindOneAndModifyOptions> options);

private:
    
    /// Returns a document of database name and collection name
    nlohmann::json m_base_operation_args {
        { "database" , database_name },
        { "collection" , name }
    };

};

} // namespace app
} // namespace realm

#endif /* remote_mongo_collection_h */

