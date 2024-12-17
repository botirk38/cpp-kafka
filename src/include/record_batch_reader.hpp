#include "byte_reader.hpp"

class RecordReader : public ByteReader<RecordReader> {
public:
  struct Record {
    int32_t length;
    int8_t attributes;
    int32_t timestamp_delta;
    int32_t offset_delta;
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;
    int32_t headers_count;
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
      record.key.clear();
      record.key.reserve(key_length);
      for (int i = 0; i < key_length; i++) {
        uint8_t byte;
        readRaw(byte);
        record.key.push_back(byte);
      }
    }

    int32_t value_length;
    readZigZagVarint(value_length);
    if (value_length >= 0) {
      record.value.clear();
      record.value.reserve(value_length);
      for (int i = 0; i < value_length; i++) {
        uint8_t byte;
        readRaw(byte);
        record.value.push_back(byte);
      }
    }
    return *this;
  }

  RecordReader &readHeaders() { return readVarint(record.headers_count); }

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
    int32_t crc;
    int16_t attributes;
    int32_t last_offset_delta;
    int64_t base_timestamp;
    int64_t max_timestamp;
    int64_t producer_id;
    int16_t producer_epoch;
    int32_t base_sequence;
    int32_t records_count;
    std::vector<Record> records;
  };

  explicit RecordBatchReader(std::ifstream &file) : ByteReader(file) {}

  RecordBatchReader &readHeader() {
    return readNetworkOrder(batch.base_offset)
        .readNetworkOrder(batch.batch_length)
        .readNetworkOrder(batch.partition_leader_epoch)
        .readRaw(batch.magic_byte)
        .readNetworkOrder(batch.crc);
  }

  RecordBatchReader &readMetadata() {
    return readNetworkOrder(batch.attributes)
        .readNetworkOrder(batch.last_offset_delta)
        .readNetworkOrder(batch.base_timestamp)
        .readNetworkOrder(batch.max_timestamp)
        .readNetworkOrder(batch.producer_id)
        .readNetworkOrder(batch.producer_epoch)
        .readNetworkOrder(batch.base_sequence)
        .readNetworkOrder(batch.records_count);
  }

  RecordBatchReader &readRecords() {
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
