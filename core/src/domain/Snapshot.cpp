#include "serpent/domain/Snapshot.h"
#include "serpent/domain/ObjectGraph.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <msgpack.hpp>
#include <zstd.h>
#include "serpent/domain/PyObjectNode.h"

namespace serpent {
namespace domain {

Snapshot::Snapshot(uint64_t epoch_ms, uint32_t py_major, uint32_t py_minor, ObjectGraph graph)
    : _object_graph(std::move(graph)) {
    _header.epoch_ms = epoch_ms;
    _header.py_major = py_major;
    _header.py_minor = py_minor;
    _header.node_count = static_cast<uint32_t>(_object_graph.nodes().size());
    std::cout << "Snapshot created. Epoch: " << epoch_ms << ", Nodes: " << _header.node_count << std::endl;
}

bool Snapshot::serialize(const std::string& filepath) const {
    std::cout << "Serializing snapshot to: " << filepath << std::endl;
    try {
        // 1. Serialize ObjectGraph to msgpack buffer
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pk(&sbuf);
        pk.pack_map(_object_graph.nodes().size());
        for (const auto& pair : _object_graph.nodes()) {
            pk.pack(pair.first); // addr (uint64_t)
            pk.pack(pair.second); // PyObjectNode
        }


        // 2. Compress the msgpack buffer using zstd
        size_t const cBuffSize = ZSTD_compressBound(sbuf.size());
        std::vector<char> compressed_buffer(cBuffSize);
        size_t const cSize = ZSTD_compress(compressed_buffer.data(), cBuffSize, sbuf.data(), sbuf.size(), 1); // Use default compression level

        if (ZSTD_isError(cSize)) {
            std::cerr << "ZSTD compression error: " << ZSTD_getErrorName(cSize) << std::endl;
            return false;
        }
        compressed_buffer.resize(cSize); // Resize to actual compressed size

        // 3. Write header and compressed data to file
        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return false;
        }

        ofs.write(reinterpret_cast<const char*>(&_header), sizeof(SnapshotHeader));
        ofs.write(compressed_buffer.data(), compressed_buffer.size());

        std::cout << "Snapshot serialized successfully. Original size: " << sbuf.size()
                  << ", Compressed size: " << cSize << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Serialization error: " << e.what() << std::endl;
        return false;
    }
}

std::unique_ptr<Snapshot> Snapshot::deserialize(const std::string& filepath) {
    std::cout << "Deserializing snapshot from: " << filepath << std::endl;
    try {
        std::ifstream ifs(filepath, std::ios::binary);
        if (!ifs) {
            std::cerr << "Failed to open file for reading: " << filepath << std::endl;
            return nullptr;
        }

        // 1. Read header
        SnapshotHeader header;
        ifs.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));
        if (ifs.gcount() != sizeof(SnapshotHeader)) {
             std::cerr << "Failed to read snapshot header." << std::endl;
             return nullptr;
        }


        // 2. Read compressed data
        ifs.seekg(0, std::ios::end);
        size_t compressed_size = ifs.tellg();
        compressed_size -= sizeof(SnapshotHeader); // Subtract header size
        ifs.seekg(sizeof(SnapshotHeader), std::ios::beg);

        std::vector<char> compressed_buffer(compressed_size);
        ifs.read(compressed_buffer.data(), compressed_size);
         if (ifs.gcount() != static_cast<std::streamsize>(compressed_size)) {
             std::cerr << "Failed to read compressed snapshot data." << std::endl;
             return nullptr;
        }

        // 3. Decompress data using zstd
        unsigned long long const rBuffSize = ZSTD_getFrameContentSize(compressed_buffer.data(), compressed_size);
        if (rBuffSize == ZSTD_CONTENTSIZE_ERROR || rBuffSize == ZSTD_CONTENTSIZE_UNKNOWN) {
            std::cerr << "ZSTD: Unable to get decompressed size or error." << std::endl;
            return nullptr;
        }

        std::vector<char> decompressed_buffer(rBuffSize);
        size_t const dSize = ZSTD_decompress(decompressed_buffer.data(), rBuffSize, compressed_buffer.data(), compressed_size);

        if (ZSTD_isError(dSize)) {
            std::cerr << "ZSTD decompression error: " << ZSTD_getErrorName(dSize) << std::endl;
            return nullptr;
        }
        if (dSize != rBuffSize) {
            std::cerr << "ZSTD decompression error: size mismatch." << std::endl;
            return nullptr;
        }


        // 4. Deserialize ObjectGraph from msgpack buffer
        ObjectGraph graph; // Create an empty graph
        msgpack::object_handle oh = msgpack::unpack(decompressed_buffer.data(), dSize);
        msgpack::object obj = oh.get();

        // Assuming the ObjectGraph's NodeMap was packed as a msgpack map
        if (obj.type == msgpack::type::MAP) {
            for (uint32_t i = 0; i < obj.via.map.size; ++i) {
                uint64_t addr;
                PyObjectNode node_data;
                obj.via.map.ptr[i].key.convert(addr);
                obj.via.map.ptr[i].val.convert(node_data); // Requires PyObjectNode to have convert method or MSGPACK_DEFINE_ARRAY
                graph.nodes()[addr] = node_data;
            }
        } else {
            std::cerr << "Deserialization error: Expected msgpack map for ObjectGraph nodes." << std::endl;
            return nullptr;
        }


        auto snapshot = std::make_unique<Snapshot>(header.epoch_ms, header.py_major, header.py_minor, std::move(graph));
        // The constructor already sets the node_count, but we can verify
        if (snapshot->_header.node_count != header.node_count) {
             std::cerr << "Warning: Deserialized node count mismatch. Header: " << header.node_count << ", Actual: " << snapshot->_header.node_count << std::endl;
             // Potentially update or flag this. For now, constructor's count from actual graph size is likely more accurate.
        }
        snapshot->_header = header; // Ensure all header fields are from the file.

        std::cout << "Snapshot deserialized successfully. Nodes: " << snapshot->getGraph().nodes().size() << std::endl;
        return snapshot;

    } catch (const std::exception& e) {
        std::cerr << "Deserialization error: " << e.what() << std::endl;
        return nullptr;
    }
}

} // namespace domain
} // namespace serpent
