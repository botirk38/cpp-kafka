#pragma once
#include "byte_reader.hpp"
#include <iostream>

class RecordReader : public ByteReader<RecordReader> {
public:
  struct Header {
    std::string key;
    std::vector<uint8_t> value;
  };

  struct Record {
    int32_t length;
    int8_t attributes;
    int32_t timestamp_delta;
    int32_t offset_delta;
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;
    std::vector<Header> headers;
  };

  explicit RecordReader(std::ifstream &file) : ByteReader(file) {}

  RecordReader &readLength() { return readVarint(record.length); }

  RecordReader &readAttributes() { return readRaw(record.attributes); }

  RecordReader &readTimestampDelta() {
    return readVarint(record.timestamp_delta);
  }

  RecordReader &readOffsetDelta() { return readVarint(record.offset_delta); }

  RecordReader &readKeyValue() {
    int32_t key_length;
    readZigZagVarint(key_length);
    if (key_length >= 0) {
      record.key.resize(key_length);
      readBytes(record.key, key_length);
    }

    int32_t value_length;
    readZigZagVarint(value_length);
    if (value_length >= 0) {
      record.value.resize(value_length);
      readBytes(record.value, value_length);
    }
    return *this;
  }

  RecordReader &readHeaders() {
    int32_t headers_count;
    readVarint(headers_count);

    for (int i = 0; i < headers_count; i++) {
      Header header;

      // Read header key
      int32_t key_length;
      readVarint(key_length);
      std::vector<char> key_chars(key_length);
      readBytes(reinterpret_cast<uint8_t *>(key_chars.data()), key_length);
      header.key = std::string(key_chars.begin(), key_chars.end());

      // Read header value
      int32_t value_length;
      readVarint(value_length);
      if (value_length >= 0) {
        header.value.resize(value_length);
        readBytes(header.value, value_length);
      }

      record.headers.push_back(std::move(header));
    }
    return *this;
  }
  Record complete() { return std::move(record); }

private:
  Record record;
};

class RecordBatchReader : public ByteReader<RecordBatchReader> {
public:
  using Record = RecordReader::Record;

  struct RecordBatch {
    int64_t base_offset;
    int32_t batch_length;
    int32_t partition_leader_epoch;
    int8_t magic_byte;
    uint32_t crc;
    int16_t attributes;
    int32_t last_offset_delta;
    int64_t base_timestamp;
    int64_t max_timestamp;
    int64_t producer_id;
    int16_t producer_epoch;
    int32_t base_sequence;
    int32_t records_count;

    std::vector<Record> records;

    std::vector<uint8_t> raw_data;
    size_t raw_data_size;
  };

  explicit RecordBatchReader(std::ifstream &file) : ByteReader(file) {}

  RecordBatchReader &readHeader() {
    // Store starting position
    auto start_pos = file.tellg();

    readInt64(batch.base_offset)
        .readInt32(batch.batch_length)
        .readInt32(batch.partition_leader_epoch)
        .readRaw(batch.magic_byte);
    readUint32(batch.crc);

    // Store raw data
    auto current_pos = file.tellg();
    file.seekg(start_pos);

    batch.raw_data.resize(batch.batch_length + 12); // Include header size
    file.read(reinterpret_cast<char *>(batch.raw_data.data()),
              batch.batch_length + 12);
    batch.raw_data_size = batch.batch_length + 12;

    // Restore position
    file.seekg(current_pos);

    std::cout << "CRC Batch: " << std::hex << batch.crc << std::dec
              << std::endl;
    return *this;
  }

  RecordBatchReader &readMetadata() {
    return readInt16(batch.attributes)
        .readInt32(batch.last_offset_delta)
        .readInt64(batch.base_timestamp)
        .readInt64(batch.max_timestamp)
        .readInt64(batch.producer_id)
        .readInt16(batch.producer_epoch)
        .readInt32(batch.base_sequence)
        .readInt32(batch.records_count);
  }

  RecordBatchReader &readRecords() {
    batch.records.reserve(batch.records_count);
    for (int i = 0; i < batch.records_count; i++) {
      batch.records.push_back(RecordReader(file)
                                  .readLength()
                                  .readAttributes()
                                  .readTimestampDelta()
                                  .readOffsetDelta()
                                  .readKeyValue()
                                  .readHeaders()
                                  .complete());
    }
    return *this;
  }

  RecordBatch complete() { return std::move(batch); }

private:
  RecordBatch batch;
};
