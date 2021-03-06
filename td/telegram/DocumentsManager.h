//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/secret_api.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/telegram/DialogId.h"
#include "td/telegram/files/FileId.h"
#include "td/telegram/Photo.h"
#include "td/telegram/SecretInputMedia.h"

#include "td/utils/buffer.h"
#include "td/utils/common.h"

#include <unordered_map>
#include <utility>

namespace td {
class MultiPromiseActor;
class Td;
}  // namespace td

namespace td {

class DocumentsManager {
 public:
  explicit DocumentsManager(Td *td);

  enum class DocumentType { Unknown, Animation, Audio, General, Sticker, Video, VideoNote, VoiceNote };

  class RemoteDocument {
   public:
    tl_object_ptr<telegram_api::document> document;
    // or
    tl_object_ptr<telegram_api::encryptedFile> secret_file;
    tl_object_ptr<secret_api::decryptedMessageMediaDocument> secret_document;

    vector<tl_object_ptr<telegram_api::DocumentAttribute>> attributes;

    RemoteDocument(tl_object_ptr<telegram_api::document> &&server_document)
        : document(std::move(server_document))
        , secret_file(nullptr)
        , secret_document(nullptr)
        , attributes(std::move(document->attributes_)) {
    }

    RemoteDocument(tl_object_ptr<telegram_api::encryptedFile> &&secret_file,
                   tl_object_ptr<secret_api::decryptedMessageMediaDocument> &&secret_document,
                   vector<tl_object_ptr<telegram_api::DocumentAttribute>> &&attributes)
        : document(nullptr)
        , secret_file(std::move(secret_file))
        , secret_document(std::move(secret_document))
        , attributes(std::move(attributes)) {
    }
  };

  tl_object_ptr<td_api::document> get_document_object(FileId file_id);

  std::pair<DocumentType, FileId> on_get_document(RemoteDocument remote_document, DialogId owner_dialog_id,
                                                  MultiPromiseActor *load_data_multipromise_ptr = nullptr);

  void create_document(FileId file_id, PhotoSize thumbnail, string file_name, string mime_type, bool replace);

  bool has_input_media(FileId file_id, FileId thumbnail_file_id, bool is_secret) const;

  SecretInputMedia get_secret_input_media(FileId document_file_id,
                                          tl_object_ptr<telegram_api::InputEncryptedFile> input_file,
                                          const string &caption, BufferSlice thumbnail) const;

  tl_object_ptr<telegram_api::InputMedia> get_input_media(FileId file_id,
                                                          tl_object_ptr<telegram_api::InputFile> input_file,
                                                          tl_object_ptr<telegram_api::InputFile> input_thumbnail) const;

  FileId get_document_thumbnail_file_id(FileId file_id) const;

  void delete_document_thumbnail(FileId file_id);

  FileId dup_document(FileId new_id, FileId old_id);

  bool merge_documents(FileId new_id, FileId old_id, bool can_delete_old);

  template <class T>
  void store_document(FileId file_id, T &storer) const;

  template <class T>
  FileId parse_document(T &parser);

  string get_document_search_text(FileId file_id) const;

 private:
  class Document {
   public:
    string file_name;
    string mime_type;
    PhotoSize thumbnail;
    FileId file_id;

    bool is_changed = true;
  };

  const Document *get_document(FileId file_id) const;

  FileId on_get_document(std::unique_ptr<Document> new_document, bool replace);

  Td *td_;
  std::unordered_map<FileId, unique_ptr<Document>, FileIdHash> documents_;  // file_id -> Document
};

}  // namespace td
