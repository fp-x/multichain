// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Original code was distributed under the MIT software license.
// Copyright (c) 2014-2017 Coin Sciences Ltd
// MultiChain code distributed under the GPLv3 license, see COPYING file.

#ifndef BITCOIN_TXDB_H
#define BITCOIN_TXDB_H

#include <storage/leveldbwrapper.h>
#include "core/main.h"
//#include "chain.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CCoins;
class uint256;
struct CExtDiskTxPos;

//! -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
//! max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
//! min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;

struct CDiskTxPos : public CDiskBlockPos
{
    unsigned int nTxOffset; // after header

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CDiskBlockPos*)this);
        READWRITE(VARINT(nTxOffset));
    }

    CDiskTxPos(const CDiskBlockPos &blockIn, unsigned int nTxOffsetIn) : CDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn) {
    }

    CDiskTxPos() {
        SetNull();
    }

    void SetNull() {
        CDiskBlockPos::SetNull();
        nTxOffset = 0;
    }

    friend bool operator<(const CDiskTxPos &a, const CDiskTxPos &b) {
        return (a.nFile < b.nFile || (
               (a.nFile == b.nFile) && (a.nPos < b.nPos || (
               (a.nPos == b.nPos) && (a.nTxOffset < b.nTxOffset)))));
    }
};

struct CExtDiskTxPos : public CDiskTxPos
{
    unsigned int nHeight;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
            READWRITE(*(CDiskTxPos*)this);
            READWRITE(VARINT(nHeight));
    }

    CExtDiskTxPos(const CDiskTxPos &pos, int nHeightIn) : CDiskTxPos(pos), nHeight(nHeightIn) {
    }

    CExtDiskTxPos() {
        SetNull();
    }

    void SetNull() {
        CDiskTxPos::SetNull();
        nHeight = 0;
    }

    friend bool operator==(const CExtDiskTxPos &a, const CExtDiskTxPos &b) {
        return (a.nHeight == b.nHeight && a.nFile == b.nFile && a.nPos == b.nPos && a.nTxOffset == b.nTxOffset);
    }

    friend bool operator!=(const CExtDiskTxPos &a, const CExtDiskTxPos &b) {
        return !(a == b);
    }

    friend bool operator<(const CExtDiskTxPos &a, const CExtDiskTxPos &b) {
        if (a.nHeight < b.nHeight) return true;
        if (a.nHeight > b.nHeight) return false;
        return ((const CDiskTxPos)a < (const CDiskTxPos)b);
    }
};

/** CCoinsView backed by the LevelDB coin database (chainstate/) */
class CCoinsViewDB : public CCoinsView
{
protected:
    CLevelDBWrapper db;
public:
    CCoinsViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool GetCoins(const uint256 &txid, CCoins &coins) const;
    bool HaveCoins(const uint256 &txid) const;
    uint256 GetBestBlock() const;
    bool BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock);
    bool GetStats(CCoinsStats &stats) const;
};

/** Access to the block database (blocks/index/) */
class CBlockTreeDB : public CLevelDBWrapper
{
public:
    CBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    uint256 salt;
    CBlockTreeDB(const CBlockTreeDB&);
    void operator=(const CBlockTreeDB&);
public:
    bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
    bool ReadBlockFileInfo(int nFile, CBlockFileInfo &fileinfo);
    bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteLastBlockFile(int nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CDiskTxPos &pos);
    bool WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> > &list);
    bool ReadAddrIndex(uint160 addrid, std::vector<CExtDiskTxPos> &list);
    bool AddAddrIndex(const std::vector<std::pair<uint160, CExtDiskTxPos> > &list);
    bool WriteFlag(const std::string &name, bool fValue);
    bool ReadFlag(const std::string &name, bool &fValue);
    bool LoadBlockIndexGuts();
};

#endif // BITCOIN_TXDB_H
