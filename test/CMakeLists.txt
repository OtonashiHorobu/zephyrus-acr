# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright (c) 2026 Otonashi Horobu

cmrc_add_resource_library(zephyrus-asset-test
    ALIAS Zephyrus::Asset::Test
    NAMESPACE zephyrus::asset::test
    WHENCE asset
    asset/hello.txt
    asset/empty.txt
    asset/binary.bin
    asset/dir/nested.txt
    asset/dir/deep/leaf.txt
)
