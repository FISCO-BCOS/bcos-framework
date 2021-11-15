![](https://github.com/FISCO-BCOS/FISCO-BCOS/raw/master/docs/images/FISCO_BCOS_Logo.svg?sanitize=true)

English / [中文](doc/README_CN.md)
# bcos-framework

[![codecov](https://codecov.io/gh/FISCO-BCOS/bcos-framework/branch/master/graph/badge.svg)](https://codecov.io/gh/FISCO-BCOS/bcos-framework)
[![CodeFactor](https://www.codefactor.io/repository/github/fisco-bcos/bcos-framework/badge)](https://www.codefactor.io/repository/github/fisco-bcos/bcos-framework)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/08552871ee104fe299b00bc79f8a12b9)](https://www.codacy.com/app/fisco-dev/FISCO-BCOS?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=FISCO-BCOS/bcos-framework&amp;utm_campaign=Badge_Grade)
[![GitHub All Releases](https://img.shields.io/github/downloads/FISCO-BCOS/bcos-framework/total.svg)](https://github.com/FISCO-BCOS/bcos-framework)
[![Code Lines](https://tokei.rs/b1/github/FISCO-BCOS/bcos-framework?category=code)](https://github.com/FISCO-BCOS/bcos-framework)
[![version](https://img.shields.io/github/tag/FISCO-BCOS/bcos-framework.svg)](https://github.com/FISCO-BCOS/bcos-framework/releases/latest)


bcos-framework is the basic framework of [FISCO BCOS 3.0](https://github.com/FISCO-BCOS/bcos-tars-services). On the one hand, it implements the basic dependency library and on the other hand defines the interface of each module, the corresponding relationship between the module interface defined by bcos-framework and its corresponding project implementation is as follows:

- **[Cryptography Interfaces](./interfaces/crypto/)**: The main functions include signature, signature verification, hashing, symmetric encryption and decryption, etc. Please refer to [bcos-crypto](https://github.com/FISCO-BCOS/bcos-crypto)

- **[Blockchain basic data protocol interfaces](./interfaces/protocol/)**: Defines the read and write interfaces related to the basic data structure of blocks, transactions, receipts, etc. Please refer to [bcos-tars-protocol](https://github.com/FISCO-BCOS/bcos-tars-protocol) for implementation

- **[Storage Interfaces](./interfaces/storage/)**: Defines the data structure and read and write interfaces of the storage module. Please refer to [bcos-storage](https://github.com/FISCO-BCOS/bcos-storage) for implementation

- **[Execution Interfaces](./interfaces/executor/)**: defines the interface for transaction execution, please refer to [bcos-executor](https://github.com/FISCO-BCOS/bcos-executor)

- **[Execution Scheduling Interfaces](./interfaces/dispatcher/)**: Parallel scheduling of transactions in the block to multiple executors, please refer to [bcos-scheduler](https://github.com/FISCO-BCOS/bcos-scheduler)

- **[Node Network Front Interfaces](./interfaces/front/)**: The network function interface of the blockchain node, all nodes call the module interface to send and receive network messages, please refer to [bcos-front](https://github.com/FISCO-BCOS/bcos-front) for function realization

- **[Gateway Interfaces](./interfaces/gateway/)**: Responsible for routing between blockchain nodes, a blockchain gateway can connect to multiple nodes, please refer to [bcos-gateway]( https://github.com/FISCO-BCOS/bcos-gateway)

- **[Transaction Pool Interfaces](./interfaces/txpool/)**: Responsible for transaction verification and transaction synchronization, please refer to [bcos-txpool](https://github.com/FISCO-BCOS/bcos-txpool)

- **[Block Sync Interfaces](./interfaces/sync/)**: Responsible for block synchronization, please refer to [bcos-sync](https://github.com/FISCO-BCOS/bcos-sync)

- **[Sealer Interfaces](./interfaces/sealer/)**: Responsible for packaging batches of transactions into blocks, please refer to [libsealer](./libsealer) for function implementation

- **[Consensus Interfaces](./interfaces/consensus/)**: Responsible for batch consensus on sorted blocks, batch consensus on transaction execution results, and write consensus blocks into blocks Chain ledger, please refer to [bcos-pbft](https://github.com/FISCO-BCOS/bcos-pbft) for function implementation

- **[Ledger Interfaces](./interfaces/ledger/)**: Defines the basic interface for accessing blockchain ledger data. For function implementation, please refer to [bcos-ledger](https://github.com/FISCO-BCOS/bcos-ledger)

- **[RPC Module Interfaces](./interfaces/rpc/)**: The service exposed by the blockchain node, which can provide RPC services for multiple nodes. Please refer to [bcos-rpc](https://github.com/FISCO-BCOS/bcos-rpc)

## Documentation

- [FISCO BCOS 3.0 quick start](https://fisco-bcos-documentation.readthedocs.io/zh_CN/latest/docs/installation.html)
- [FISCO BCOS 3.0 system design](https://TODO.html)

## Code contribution

- Your contributions are most welcome and appreciated. Please read the [contribution instructions](https://mp.weixin.qq.com/s/_w_auH8X4SQQWO3lhfNrbQ).
- If this project is useful to you, please star us on GitHub project page!

## Join Our Community

The FISCO BCOS community is one of the most active open-source blockchain communities in China. It provides long-term technical support for both institutional and individual developers and users of FISCO BCOS. Thousands of technical enthusiasts from numerous industry sectors have joined this community, studying and using FISCO BCOS platform. If you are also interested, you are most welcome to join us for more support and fun.

![](https://raw.githubusercontent.com/FISCO-BCOS/LargeFiles/master/images/QR_image_en.png)

## License

[![](https://img.shields.io/github/license/FISCO-BCOS/bcos-framework.svg)](./LICENSE)

All contributions are made under the Apache License, See [LICENSE](./LICENSE).