// SPDX-License-Identifier: MIT
//
// SPDX-FileCopyrightText: Copyright(c) 2026 Otonashi Horobu

#include <zephyrus/log.hpp>
#include <zephyrus/log/logging_manager.hpp>
#include <zephyrus/rmlui/cmrc_file_interface.hpp>

#include <atomic>
#include <thread>
#include <vector>

#include <boost/ut.hpp>
#include <cmrc/cmrc.hpp>
#include <fmt/format.h>

CMRC_DECLARE(zephyrus::asset::test);

int main() {
    using namespace boost::ut;
    using namespace std::literals;
    using zephyrus::rmlui::cmrc_file_interface;

    auto fs{cmrc::zephyrus::asset::test::get_filesystem()};
    constexpr int thread_count{8};

    zephyrus::log::logging_manager logging_manager{};
    if (auto result{logging_manager.init()}; !result) {
        fmt::println("zephyrus: failed to initialize logging system: {}",
                     result.error());
        return 1;
    }

    "open/existing file returns nonzero handle"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);
        iface.Close(handle);
    };

    "open/nonexistent file returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("no_such_file.txt")};
        expect(handle == 0);
    };

    "open/same file twice returns distinct handles"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto h1{iface.Open("hello.txt")};
        auto h2{iface.Open("hello.txt")};
        expect(h1 != 0);
        expect(h2 != 0);
        expect(h1 != h2);
        iface.Close(h1);
        iface.Close(h2);
    };

    "open/empty string path returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("")};
        expect(handle == 0);
    };

    "open/path traversal parent ref returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        expect(iface.Open("../hello.txt") == 0);
    };

    "open/path traversal deep returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        expect(iface.Open("../../etc/passwd") == 0);
    };

    "open/path traversal dotdot slash returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        expect(iface.Open("dir/../../hello.txt") == 0);
    };

    "close/valid handle does not crash"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);
        iface.Close(handle);
    };

    "close/double close does not crash"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);
        iface.Close(handle);
        iface.Close(handle);
    };

    "close/null handle does not crash"_test = [&fs] {
        cmrc_file_interface iface{fs};
        iface.Close(0);
    };

    "close/many handles for same file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        constexpr int count{16};
        Rml::FileHandle handles[count]{};
        for (int i{}; i < count; ++i) {
            handles[i] = iface.Open("hello.txt");
            expect(handles[i] != 0);
        }
        for (int i{}; i < count; ++i) {
            iface.Close(handles[i]);
        }
    };

    "close/invalid handle does not crash"_test = [&fs] {
        cmrc_file_interface iface{fs};
        iface.Close(static_cast<Rml::FileHandle>(0xDEAD));
    };

    "read/after close returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        iface.Close(handle);
        char buf[16]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 0_u);
    };

    "read/full file content"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[64]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 13_u);
        expect(std::string_view{buf, n} == "Hello, world!"sv);
        iface.Close(handle);
    };

    "read/with small buffer"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[4]{};
        auto n1{iface.Read(buf, sizeof(buf), handle)};
        expect(n1 == 4_u);
        expect(std::string_view(buf, n1) == "Hell"sv);
        auto n2{iface.Read(buf, sizeof(buf), handle)};
        expect(n2 == 4_u);
        expect(std::string_view(buf, n2) == "o, w"sv);
        iface.Close(handle);
    };

    "read/at eof returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[16]{};
        iface.Read(buf, sizeof(buf), handle);
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 0_u);
        iface.Close(handle);
    };

    "read/with invalid handle returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        char buf[16]{};
        auto n =
            iface.Read(buf, sizeof(buf), static_cast<Rml::FileHandle>(0xDEAD));
        expect(n == 0_u);
    };

    "read/empty file returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("empty.txt")};
        expect(handle != 0);
        char buf[16]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 0_u);
        iface.Close(handle);
    };

    "read/advances cursor"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[5]{};
        iface.Read(buf, sizeof(buf), handle);
        expect(iface.Tell(handle) == 5_u);
        iface.Close(handle);
    };

    "read/subdirectory file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("dir/nested.txt")};
        expect(handle != 0);
        char buf[64]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 14_u);
        expect(std::string_view{buf, n} == "nested content"sv);
        iface.Close(handle);
    };

    "read/deeply nested file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("dir/deep/leaf.txt")};
        expect(handle != 0);
        char buf[64]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 9_u);
        expect(std::string_view{buf, n} == "deep leaf"sv);
        iface.Close(handle);
    };

    "read/binary file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("binary.bin")};
        expect(handle != 0);
        char buf[16]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 4_u);
        expect(buf[0] == '\x00');
        expect(buf[1] == '\x01');
        expect(buf[2] == '\x02');
        expect(buf[3] == '\xFF');
        iface.Close(handle);
    };

    "read/with null buffer returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);
        auto n{iface.Read(nullptr, 16, handle)};
        expect(n == 0_u);
        iface.Close(handle);
    };

    "read/with zero size returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);
        char buf[16]{};
        auto n{iface.Read(buf, 0, handle)};
        expect(n == 0_u);
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "read/after seek to beginning"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[16]{};
        iface.Read(buf, sizeof(buf), handle);
        expect(iface.Tell(handle) == 13_u);
        expect(iface.Seek(handle, 0, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 13_u);
        expect(std::string_view{buf, n} == "Hello, world!"sv);
        iface.Close(handle);
    };

    "read/after seek to end returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 0, SEEK_END));
        char buf[16]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 0_u);
        iface.Close(handle);
    };

    "read/binary file byte by byte"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("binary.bin")};
        expect(handle != 0);
        const unsigned char expected[] = {0x00, 0x01, 0x02, 0xFF};
        char c{};
        for (std::size_t i{}; i < 4; ++i) {
            auto n{iface.Read(&c, 1, handle)};
            expect(n == 1_u);
            expect(static_cast<unsigned char>(c) == expected[i]);
        }
        expect(iface.Read(&c, 1, handle) == 0_u);
        iface.Close(handle);
    };

    "read/two reads split at boundary"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[13]{};
        auto n1{iface.Read(buf, 6, handle)};
        expect(n1 == 6_u);
        expect(std::string_view{buf, n1} == "Hello,"sv);
        auto n2{iface.Read(buf + 6, 7, handle)};
        expect(n2 == 7_u);
        expect(std::string_view{buf + 6, n2} == " world!"sv);
        expect(iface.Read(buf, 1, handle) == 0_u);
        iface.Close(handle);
    };

    "read/advances cursor correctly across multiple reads"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[4]{};
        iface.Read(buf, 2, handle);
        expect(iface.Tell(handle) == 2_u);
        iface.Read(buf, 1, handle);
        expect(iface.Tell(handle) == 3_u);
        iface.Read(buf, 3, handle);
        expect(iface.Tell(handle) == 6_u);
        iface.Close(handle);
    };

    "seek/set"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 5, SEEK_SET));
        expect(iface.Tell(handle) == 5_u);
        iface.Close(handle);
    };

    "seek/cur"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        char buf[3]{};
        iface.Read(buf, sizeof(buf), handle);
        expect(iface.Tell(handle) == 3_u);
        expect(iface.Seek(handle, 2, SEEK_CUR));
        expect(iface.Tell(handle) == 5_u);
        iface.Close(handle);
    };

    "seek/end"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, -3, SEEK_END));
        expect(iface.Tell(handle) == 10_u);
        iface.Close(handle);
    };

    "seek/past beginning clamps to zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, -100, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "seek/past end clamps to file size"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 100, SEEK_SET));
        expect(iface.Tell(handle) == 13_u);
        iface.Close(handle);
    };

    "seek/with invalid origin returns false"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(!iface.Seek(handle, 0, 999));
        iface.Close(handle);
    };

    "seek/with invalid handle returns false"_test = [&fs] {
        cmrc_file_interface iface{fs};
        expect(!iface.Seek(static_cast<Rml::FileHandle>(0xDEAD), 0, SEEK_SET));
    };

    "seek/zero from beginning stays at zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 0, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "seek/multiple sequential seeks"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 5, SEEK_SET));
        expect(iface.Tell(handle) == 5_u);
        expect(iface.Seek(handle, 3, SEEK_CUR));
        expect(iface.Tell(handle) == 8_u);
        expect(iface.Seek(handle, -2, SEEK_END));
        expect(iface.Tell(handle) == 11_u);
        expect(iface.Seek(handle, 0, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "seek/on empty file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("empty.txt")};
        expect(handle != 0);
        expect(iface.Seek(handle, 0, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        expect(iface.Seek(handle, 5, SEEK_SET));
        expect(iface.Tell(handle) == 0_u);
        expect(iface.Seek(handle, -5, SEEK_END));
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "seek/negative cur backwards"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 10, SEEK_SET));
        expect(iface.Tell(handle) == 10_u);
        expect(iface.Seek(handle, -5, SEEK_CUR));
        expect(iface.Tell(handle) == 5_u);
        iface.Close(handle);
    };

    "seek/then read consistent"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Seek(handle, 7, SEEK_SET));
        char buf[6]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 6_u);
        expect(std::string_view{buf, n} == "world!"sv);
        iface.Close(handle);
    };

    "tell/with invalid handle returns zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        expect(iface.Tell(static_cast<Rml::FileHandle>(0xDEAD)) == 0_u);
    };

    "tell/at file start is zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "tell/after seek and read"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        iface.Seek(handle, 5, SEEK_SET);
        char buf[3]{};
        iface.Read(buf, 3, handle);
        expect(iface.Tell(handle) == 8_u);
        iface.Close(handle);
    };

    "tell/on empty file is zero"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("empty.txt")};
        expect(handle != 0);
        expect(iface.Tell(handle) == 0_u);
        iface.Close(handle);
    };

    "lifecycle/destructor cleans up open files"_test = [&fs] {
        auto *iface{new cmrc_file_interface{fs}};
        iface->Open("hello.txt");
        iface->Open("dir/nested.txt");
        delete iface;
    };

    "lifecycle/full lifecycle"_test = [&fs] {
        cmrc_file_interface iface{fs};

        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);

        char buf[5]{};
        auto n{iface.Read(buf, sizeof(buf), handle)};
        expect(n == 5_u);
        expect(std::string_view{buf, n} == "Hello"sv);
        expect(iface.Tell(handle) == 5_u);

        expect(iface.Seek(handle, -3, SEEK_END));
        expect(iface.Tell(handle) == 10_u);

        n = iface.Read(buf, sizeof(buf), handle);
        expect(n == 3_u);
        expect(std::string_view{buf, n} == "ld!"sv);
        expect(iface.Tell(handle) == 13_u);

        n = iface.Read(buf, sizeof(buf), handle);
        expect(n == 0_u);

        iface.Close(handle);
    };

    "lifecycle/two interfaces share filesystem"_test = [&fs] {
        cmrc_file_interface iface1{fs};
        cmrc_file_interface iface2{fs};
        auto h1{iface1.Open("hello.txt")};
        auto h2{iface2.Open("hello.txt")};
        expect(h1 != 0);
        expect(h2 != 0);
        char buf1[64]{};
        char buf2[64]{};
        auto n1{iface1.Read(buf1, sizeof(buf1), h1)};
        auto n2{iface2.Read(buf2, sizeof(buf2), h2)};
        expect(n1 == 13_u);
        expect(n2 == 13_u);
        expect(std::string_view{buf1, n1} == std::string_view{buf2, n2});
        iface1.Close(h1);
        iface2.Close(h2);
    };

    "lifecycle/open many files simultaneously"_test = [&fs] {
        cmrc_file_interface iface{fs};
        constexpr int count{32};
        Rml::FileHandle handles[count]{};
        for (int i{}; i < count; ++i) {
            handles[i] = iface.Open("hello.txt");
            expect(handles[i] != 0);
        }
        for (int i{}; i < count; ++i) {
            char buf[13]{};
            auto n{iface.Read(buf, sizeof(buf), handles[i])};
            expect(n == 13_u);
            expect(std::string_view{buf, n} == "Hello, world!"sv);
        }
        for (int i{}; i < count; ++i) {
            iface.Close(handles[i]);
        }
    };

    "thread/concurrent open close"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        const char *files[]{"hello.txt", "empty.txt", "dir/nested.txt",
                            "dir/deep/leaf.txt"};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count, i, &files] {
                auto handle{iface.Open(files[i % 4])};
                if (handle != 0) {
                    char buf[64]{};
                    iface.Read(buf, sizeof(buf), handle);
                    iface.Close(handle);
                    ++success_count;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
    };

    "thread/concurrent read same file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count] {
                auto handle{iface.Open("hello.txt")};
                expect(handle != 0);
                char buf[64]{};
                auto n{iface.Read(buf, sizeof(buf), handle)};
                if (n == 13 && std::string_view{buf, n} == "Hello, world!"sv) {
                    ++success_count;
                }
                iface.Close(handle);
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
    };

    "thread/concurrent read same handle"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);

        std::vector<std::thread> threads;
        std::atomic<size_t> total_bytes{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &total_bytes, handle] {
                char buf[4]{};
                for (;;) {
                    auto n{iface.Read(buf, sizeof(buf), handle)};
                    if (n == 0) {
                        break;
                    }
                    total_bytes += n;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(total_bytes.load() == 13_u);
        iface.Close(handle);
    };

    "thread/concurrent mixed operations"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count] {
                auto handle{iface.Open("hello.txt")};
                expect(handle != 0);

                char buf[5]{};
                auto n{iface.Read(buf, sizeof(buf), handle)};
                expect(n == 5_u);
                expect(std::string_view{buf, n} == "Hello"sv);

                expect(iface.Seek(handle, 0, SEEK_SET));
                expect(iface.Tell(handle) == 0_u);

                expect(iface.Seek(handle, -3, SEEK_END));
                expect(iface.Tell(handle) == 10_u);

                iface.Close(handle);
                ++success_count;
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
    };

    "thread/concurrent open close stress"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> total_operations{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &total_operations] {
                for (int j{}; j < 100; ++j) {
                    auto handle{iface.Open("hello.txt")};
                    expect(handle != 0);
                    char buf[13]{};
                    auto n{iface.Read(buf, sizeof(buf), handle)};
                    expect(n == 13_u);
                    iface.Close(handle);
                    ++total_operations;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(total_operations == thread_count * 100);
    };

    "thread/concurrent seek same handle"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);

        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count, handle, i] {
                for (int j{}; j < 50; ++j) {
                    iface.Seek(handle, i * 1, SEEK_SET);
                }
                ++success_count;
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
        expect(iface.Tell(handle) < 14_u);
        iface.Close(handle);
    };

    "thread/concurrent close same handle"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);

        std::vector<std::thread> threads;
        std::atomic<int> close_count{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &close_count, handle] {
                iface.Close(handle);
                ++close_count;
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(close_count == thread_count);
    };

    "thread/concurrent read seek same handle"_test = [&fs] {
        cmrc_file_interface iface{fs};
        auto handle{iface.Open("hello.txt")};
        expect(handle != 0);

        std::vector<std::thread> threads;
        std::atomic<int> total_ops{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &total_ops, handle] {
                for (int j{}; j < 50; ++j) {
                    char buf[4]{};
                    iface.Read(buf, sizeof(buf), handle);
                    iface.Seek(handle, 0, SEEK_SET);
                    ++total_ops;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(total_ops == thread_count * 50);
        iface.Close(handle);
    };

    "thread/concurrent operations on empty file"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count] {
                auto handle{iface.Open("empty.txt")};
                expect(handle != 0);
                char buf[16]{};
                auto n{iface.Read(buf, sizeof(buf), handle)};
                expect(n == 0_u);
                expect(iface.Seek(handle, 0, SEEK_SET));
                expect(iface.Tell(handle) == 0_u);
                iface.Close(handle);
                ++success_count;
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
    };

    "thread/concurrent read binary byte by byte"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<size_t> total_bytes{0};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &total_bytes] {
                auto handle{iface.Open("binary.bin")};
                expect(handle != 0);
                char c{};
                size_t count{};
                while (iface.Read(&c, 1, handle) == 1_u) {
                    ++count;
                }
                expect(count == 4_u);
                total_bytes += count;
                iface.Close(handle);
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(total_bytes.load() == static_cast<size_t>(thread_count * 4));
    };

    "thread/high thread count stress"_test = [&fs] {
        cmrc_file_interface iface{fs};
        constexpr int kHighThreadCount{64};
        std::vector<std::thread> threads;
        std::atomic<int> total_operations{0};

        for (int i{}; i < kHighThreadCount; ++i) {
            threads.emplace_back([&iface, &total_operations] {
                for (int j{}; j < 50; ++j) {
                    auto handle{iface.Open("hello.txt")};
                    expect(handle != 0);
                    char buf[13]{};
                    auto n{iface.Read(buf, sizeof(buf), handle)};
                    expect(n == 13_u);
                    expect(iface.Seek(handle, 0, SEEK_SET));
                    n = iface.Read(buf, sizeof(buf), handle);
                    expect(n == 13_u);
                    iface.Close(handle);
                    ++total_operations;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(total_operations == kHighThreadCount * 50);
    };

    "thread/concurrent many files"_test = [&fs] {
        cmrc_file_interface iface{fs};
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        const char *files[]{"hello.txt", "empty.txt", "binary.bin",
                            "dir/nested.txt", "dir/deep/leaf.txt"};

        for (int i{}; i < thread_count; ++i) {
            threads.emplace_back([&iface, &success_count, i, &files] {
                for (int j{}; j < 20; ++j) {
                    auto handle{iface.Open(files[(i * 20 + j) % 5])};
                    expect(handle != 0);
                    char buf[64]{};
                    iface.Read(buf, sizeof(buf), handle);
                    iface.Seek(handle, 0, SEEK_SET);
                    iface.Read(buf, sizeof(buf), handle);
                    iface.Close(handle);
                }
                ++success_count;
            });
        }

        for (auto &t : threads) {
            t.join();
        }
        expect(success_count == thread_count);
    };
}
