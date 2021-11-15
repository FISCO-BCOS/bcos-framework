![](https://github.com/FISCO-BCOS/FISCO-BCOS/raw/master/docs/images/FISCO_BCOS_Logo.svg?sanitize=true)

[English](../README.md) / 中文

# 区块链基础框架

[![codecov](https://codecov.io/gh/FISCO-BCOS/bcos-framework/branch/master/graph/badge.svg)](https://codecov.io/gh/FISCO-BCOS/bcos-framework)
[![CodeFactor](https://www.codefactor.io/repository/github/fisco-bcos/bcos-framework/badge)](https://www.codefactor.io/repository/github/fisco-bcos/bcos-framework)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/08552871ee104fe299b00bc79f8a12b9)](https://www.codacy.com/app/fisco-dev/FISCO-BCOS?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=FISCO-BCOS/bcos-framework&amp;utm_campaign=Badge_Grade)
[![GitHub All Releases](https://img.shields.io/github/downloads/FISCO-BCOS/bcos-framework/total.svg)](https://github.com/FISCO-BCOS/bcos-framework)
[![Code Lines](https://tokei.rs/b1/github/FISCO-BCOS/bcos-framework?category=code)](https://github.com/FISCO-BCOS/bcos-framework)
[![version](https://img.shields.io/github/tag/FISCO-BCOS/bcos-framework.svg)](https://github.com/FISCO-BCOS/bcos-framework/releases/latest)


bcos-framework是[FISCO BCOS 3.0](https://github.com/FISCO-BCOS/bcos-tars-services)的基础框架，一方面实现了基础依赖库，另一方面定义了各个模块接口，bcos-framework定义的模块接口及其对应的项目实现间的对应关系如下：
- **[密码学模块接口](../interfaces/crypto/)**: 主要功能包括签名、验签、哈希、对称加解密等，功能实现请参考[bcos-crypto](https://github.com/FISCO-BCOS/bcos-crypto)

- **[区块链基础数据协议模块接口](../interfaces/protocol/)**: 定义了区块、交易、回执等基本数据结构相关的读写接口，功能实现请参考[bcos-tars-protocol](https://github.com/FISCO-BCOS/bcos-tars-protocol)
- **[存储模块接口](../interfaces/storage/)**: 定义了存储模块的数据结构以及读、写接口，功能实现请参考[bcos-storage](https://github.com/FISCO-BCOS/bcos-storage)
- **[执行模块接口](../interfaces/executor/)**: 定义了交易执行的接口，功能实现请参考[bcos-executor](https://github.com/FISCO-BCOS/bcos-executor)
- **[执行调度模块接口](../interfaces/dispatcher/)**: 将区块内交易并行调度到多个执行器，功能实现请参考[bcos-scheduler](https://github.com/FISCO-BCOS/bcos-scheduler)
- **[节点网络模块接口](../interfaces/front/)**: 区块链节点的网络功能接口，所有节点均调用该模块接口收发网络消息，功能实现请参考[bcos-front](https://github.com/FISCO-BCOS/bcos-front)
- **[网关模块接口](../interfaces/gateway/)**: 负责区块链节点间路由，一个区块链网关可接入多个节点，功能实现请参考[bcos-gateway](https://github.com/FISCO-BCOS/bcos-gateway)
- **[交易池接口](../interfaces/txpool/)**: 负责交易验证、交易同步，功能实现请参考[bcos-txpool](https://github.com/FISCO-BCOS/bcos-txpool)
- **[区块同步接口](../interfaces/sync/)**: 负责区块同步，功能实现请参考[bcos-sync](https://github.com/FISCO-BCOS/bcos-sync)
- **[共识打包模块接口](../interfaces/sealer/)**: 负责将批量的交易打包成区块，功能实现请参考[libsealer](../libsealer)
- **[共识模块接口](../interfaces/consensus/)**: 负责对排序好的区块进行批量共识、对交易执行结果进行批量共识，并将达成共识的区块写入区块链账本，功能实现请参考[bcos-pbft](https://github.com/FISCO-BCOS/bcos-pbft)
- **[账本访问接口](../interfaces/ledger/)**: 定义了访问区块链账本数据的基本接口，功能实现请参考[bcos-ledger](https://github.com/FISCO-BCOS/bcos-ledger)
- **[RPC模块接口](../interfaces/rpc/)**: 区块链节点对外暴露的服务，可为多个节点提供RPC服务，功能实现请参考[bcos-rpc](https://github.com/FISCO-BCOS/bcos-rpc)

## 文档

- [FISCO BCOS 3.0快速开始](https://fisco-bcos-documentation.readthedocs.io/zh_CN/latest/docs/installation.html)
- [FISCO BCOS 3.0系统设计](https://TODO.html)

## 加入社区

FISCO BCOS开源社区是国内活跃的开源社区，社区长期为机构和个人开发者提供各类支持与帮助。已有来自各行业的数千名技术爱好者在研究和使用FISCO BCOS。如您对FISCO BCOS开源技术及应用感兴趣，欢迎加入社区获得更多支持与帮助。

![](https://raw.githubusercontent.com/FISCO-BCOS/LargeFiles/master/images/QR_image.png)


## License

[![](https://img.shields.io/github/license/FISCO-BCOS/bcos-framework.svg)](../LICENSE)

bcos-framework的开源协议为Apache License, 详情参考[LICENSE](../LICENSE).