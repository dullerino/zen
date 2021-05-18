#include <sc/sidechainTxsCommitmentBuilder.h>
#include <primitives/transaction.h>
#include <primitives/certificate.h>
#include <uint256.h>
#include <algorithm>
#include <iostream>
#include <zendoo/zendoo_mc.h>

// TODO remove when not needed anymore
#include <gtest/libzendoo_test_files.h>

#ifdef BITCOIN_TX
bool SidechainTxsCommitmentBuilder::add(const CTransaction& tx) { return true; }
bool SidechainTxsCommitmentBuilder::add(const CScCertificate& cert) { return true; }
uint256 SidechainTxsCommitmentBuilder::getCommitment() { return uint256(); }
SidechainTxsCommitmentBuilder::SidechainTxsCommitmentBuilder(): _cmt(nullptr) {}
SidechainTxsCommitmentBuilder::~SidechainTxsCommitmentBuilder(){}
#else
SidechainTxsCommitmentBuilder::SidechainTxsCommitmentBuilder(): _cmt(initPtr())
{
    assert(_cmt != nullptr);
}

const commitment_tree_t* const SidechainTxsCommitmentBuilder::initPtr()
{
    return zendoo_commitment_tree_create();
}

SidechainTxsCommitmentBuilder::~SidechainTxsCommitmentBuilder()
{
    assert(_cmt != nullptr);
    zendoo_commitment_tree_delete(const_cast<commitment_tree_t*>(_cmt));
}

bool SidechainTxsCommitmentBuilder::add_scc(const CTxScCreationOut& ccout, const BufferWithSize& bws_tx_hash, uint32_t out_idx, CctpErrorCode& ret_code)
{
    const uint256& scId = ccout.GetScId();
//    BufferWithSize bws_scid(scId.begin(), scId.size());

    const uint256& pub_key = ccout.address;
    BufferWithSize bws_pk(pub_key.begin(), pub_key.size());

    BufferWithSize bws_custom_data(nullptr, 0);
    if (!ccout.customData.empty())
    {
        bws_custom_data.data = (unsigned char*)(&ccout.customData[0]);
        bws_custom_data.len = ccout.customData.size();
    }

    BufferWithSize bws_constant(nullptr, 0);
    if(ccout.constant.is_initialized())
    {
        bws_constant.data = ccout.constant->GetDataBuffer();
        bws_constant.len = ccout.constant->GetDataSize();
    }
        
    BufferWithSize bws_cert_vk(ccout.wCertVk.GetDataBuffer(), ccout.wCertVk.GetDataSize());

    BufferWithSize bws_csw_vk(nullptr, 0);
    if(ccout.wCeasedVk.is_initialized())
    {
        bws_csw_vk.data = ccout.wCeasedVk->GetDataBuffer();
        bws_csw_vk.len = ccout.wCeasedVk->GetDataSize();
    }

#if 0
    return zendoo_commitment_tree_add_scc(const_cast<commitment_tree_t*>(_cmt),
         &bws_scid,
         ccout.nValue,
         &bws_pk,
         ccout.withdrawalEpochLength,
         &bws_custom_data,
         &bws_constant,
         &bws_cert_vk,
         &bws_mbtr_vk,
         &bws_csw_vk,
         &bws_tx_hash,
         out_idx,
         &ret_code
    );
#endif
    BufferWithSize bws_dummy(nullptr, 0);

    BitVectorElementsConfig bvcfg_dummy;
    bvcfg_dummy.bit_vector_size_bits = 0;
    bvcfg_dummy.max_compressed_byte_size = 0;
    
    return zendoo_commitment_tree_add_scc(const_cast<commitment_tree_t*>(_cmt),
         nullptr, // TODO sc_id
         ccout.nValue,
         &bws_pk,
         &bws_tx_hash,
         out_idx,
         ccout.withdrawalEpochLength,
         0, // TODO mc_btr_request_data_length,
         &bws_dummy, // TODO custom_field_elements_config
         &bvcfg_dummy, // TODO custom_bv_elements_config
         0, // TODO custom_bv_elements_config_len,
         0, // TODO btr_fee,
         0, // TODO ft_min_amount,,
         &bws_custom_data,
         nullptr, // TODO const field_t* constant
         &bws_cert_vk,
         &bws_csw_vk,
         &ret_code
    );
}

bool SidechainTxsCommitmentBuilder::add_fwt(const CTxForwardTransferOut& ccout, const BufferWithSize& bws_tx_hash, uint32_t out_idx, CctpErrorCode& ret_code)
{
    const uint256& fwtScId = ccout.GetScId();
    BufferWithSize bws_fwt_scid((unsigned char*)fwtScId.begin(), fwtScId.size());

    const uint256& fwt_pub_key = ccout.address;
    BufferWithSize bws_fwt_pk((unsigned char*)fwt_pub_key.begin(), fwt_pub_key.size());

#if 0
    return zendoo_commitment_tree_add_fwt(const_cast<commitment_tree_t*>(_cmt),
         &bws_fwt_scid,
         ccout.nValue,
         &bws_fwt_pk,
         &bws_tx_hash,
         out_idx,
         &ret_code
    );
#endif
    return zendoo_commitment_tree_add_fwt(const_cast<commitment_tree_t*>(_cmt),
         nullptr, // TODO sc_id
         ccout.nValue,
         &bws_fwt_pk,
         &bws_tx_hash,
         out_idx,
         &ret_code
    );

}

bool SidechainTxsCommitmentBuilder::add_bwtr(const CBwtRequestOut& ccout, const BufferWithSize& bws_tx_hash, uint32_t out_idx, CctpErrorCode& ret_code)
{
    const uint256& bwtrScId = ccout.GetScId();
    BufferWithSize bws_bwtr_scid(bwtrScId.begin(), bwtrScId.size());

    const uint160& bwtr_pk_hash = ccout.mcDestinationAddress;
    BufferWithSize bws_bwtr_pk_hash(bwtr_pk_hash.begin(), bwtr_pk_hash.size());

    // TODO this will be changed on cctp lib in future, as of now it is a single field element so we choose to use the first,
    // since this vector can not be empty
    BufferWithSize bws_req_data(ccout.vScRequestData.at(0).GetDataBuffer(), ccout.vScRequestData.at(0).GetDataSize());
        
#if 0
    return zendoo_commitment_tree_add_bwtr(const_cast<commitment_tree_t*>(_cmt),
         &bws_bwtr_scid,
         ccout.scFee,
         &bws_req_data,
         &bws_bwtr_pk_hash,
         &bws_tx_hash,
         out_idx,
         &ret_code
    );
#endif
    return zendoo_commitment_tree_add_bwtr(const_cast<commitment_tree_t*>(_cmt),
         nullptr, // TODO sc_id
         ccout.scFee,
         nullptr, // TODO const field_t** sc_req_data
         0, // TODO sc_req_data_len,
         &bws_bwtr_pk_hash,
         &bws_tx_hash,
         out_idx,
         &ret_code
    );
}

bool SidechainTxsCommitmentBuilder::add_csw(const CTxCeasedSidechainWithdrawalInput& ccin, CctpErrorCode& ret_code)
{
    const uint256& cswScId = ccin.scId;
    BufferWithSize bws_csw_scid(cswScId.begin(), cswScId.size());

    const uint160& csw_pk_hash = ccin.pubKeyHash;
    BufferWithSize bws_csw_pk_hash(csw_pk_hash.begin(), csw_pk_hash.size());

    BufferWithSize bws_nullifier(ccin.nullifier.GetDataBuffer(), ccin.nullifier.GetDataSize());
        
    // TODO - they are not optional; for the time being set to a non empty field element
    const CFieldElement& dumFe = CFieldElement{SAMPLE_FIELD}; // libzendoo_test_files.h 
    BufferWithSize bws_active_cert_data_hash( dumFe.GetDataBuffer(), dumFe.GetDataSize());

#if 0
    return zendoo_commitment_tree_add_csw(const_cast<commitment_tree_t*>(_cmt),
         &bws_csw_scid,
         ccin.nValue,
         &bws_nullifier,
         &bws_csw_pk_hash,
         &bws_active_cert_data_hash,
         &ret_code
    );
#endif
    return zendoo_commitment_tree_add_csw(const_cast<commitment_tree_t*>(_cmt),
         nullptr, // TODO sc_id
         ccin.nValue,
         nullptr, // TODO nullifier,
         &bws_csw_pk_hash,
         &ret_code
    );
}

bool SidechainTxsCommitmentBuilder::add_cert(const CScCertificate& cert, CctpErrorCode& ret_code)
{
    const uint256& certScId = cert.GetScId();
    BufferWithSize bws_cert_scid(certScId.begin(), certScId.size());

    const CFieldElement& cdh = cert.GetDataHash(); 
    const BufferWithSize bws_cert_data_hash(cdh.GetDataBuffer(), cdh.GetDataSize());

    const backward_transfer_t* bt_list =  nullptr;
    std::vector<backward_transfer_t> vbt_list;
    for(int pos = cert.nFirstBwtPos; pos < cert.GetVout().size(); ++pos)
    {
        const CTxOut& out = cert.GetVout()[pos];
        const auto& bto = CBackwardTransferOut(out);
        backward_transfer_t x;
        x.amount = bto.nValue;
        memcpy(x.pk_dest, bto.pubKeyHash.begin(), sizeof(x.pk_dest));
        vbt_list.push_back(x);
    }

    if (!vbt_list.empty())
        bt_list = (const backward_transfer_t*)&vbt_list[0];

    size_t bt_list_len = vbt_list.size();

    // TODO - they are not optional; for the time being set to a non empty field element
    const CFieldElement& dumFe = CFieldElement{SAMPLE_FIELD}; // libzendoo_test_files.h 
    const BufferWithSize bws_custom_fields_merkle_root( dumFe.GetDataBuffer(), dumFe.GetDataSize());
    const BufferWithSize bws_end_cum_comm_tree_root( dumFe.GetDataBuffer(), dumFe.GetDataSize());
#if 0            
    return zendoo_commitment_tree_add_cert(const_cast<commitment_tree_t*>(_cmt),
         &bws_cert_scid,
         cert.epochNumber,
         cert.quality,
         &bws_cert_data_hash,
         bt_list,
         bt_list_len,
         &bws_custom_fields_merkle_root,
         &bws_end_cum_comm_tree_root,
         &ret_code
    );
#endif
    return zendoo_commitment_tree_add_cert(const_cast<commitment_tree_t*>(_cmt),
         nullptr, // TODO sc_id
         cert.epochNumber,
         cert.quality,
         bt_list,
         bt_list_len,
         nullptr, // TODO custom_fields,
         0, // TODO custom_fields_len,
         nullptr, // TODO end_cum_comm_tree_root,
         0, // TODO btr_fee,
         0, // TODO ft_min_amount,,
         &ret_code
    );
}

bool SidechainTxsCommitmentBuilder::add(const CTransaction& tx)
{
    assert(_cmt != nullptr);

    if (!tx.IsScVersion())
        return true;

    CctpErrorCode ret_code = CctpErrorCode::OK;

    const uint256& tx_hash = tx.GetHash();
    const BufferWithSize bws_tx_hash(tx_hash.begin(), tx_hash.size());

    uint32_t out_idx = 0;

    for (unsigned int scIdx = 0; scIdx < tx.GetVscCcOut().size(); ++scIdx)
    {
        const CTxScCreationOut& ccout = tx.GetVscCcOut().at(scIdx);

        if (!add_scc(ccout, bws_tx_hash, out_idx, ret_code))
        {
            LogPrintf("%s():%d Error adding sc creation: tx[%s], pos[%d], ret_code[%d]\n", __func__, __LINE__,
                tx_hash.ToString(), scIdx, ret_code);
            return false;
        }
        out_idx++;
    }

    for (unsigned int fwtIdx = 0; fwtIdx < tx.GetVftCcOut().size(); ++fwtIdx)
    {
        const CTxForwardTransferOut& ccout = tx.GetVftCcOut().at(fwtIdx);

        if (!add_fwt(ccout, bws_tx_hash, out_idx, ret_code))
        {
            LogPrintf("%s():%d Error adding fwt: tx[%s], pos[%d], ret_code[%d]\n", __func__, __LINE__,
                tx_hash.ToString(), fwtIdx, ret_code);
            return false;
        }
        out_idx++;
    }

    for (unsigned int bwtrIdx = 0; bwtrIdx < tx.GetVBwtRequestOut().size(); ++bwtrIdx)
    {
        const CBwtRequestOut& ccout = tx.GetVBwtRequestOut().at(bwtrIdx);

        if (!add_bwtr(ccout, bws_tx_hash, out_idx, ret_code))
        {
            LogPrintf("%s():%d Error adding bwtr: tx[%s], pos[%d], ret_code[%d]\n", __func__, __LINE__,
                tx_hash.ToString(), bwtrIdx, ret_code);
            return false;
        }
 
        out_idx++;
    }

    for (unsigned int cswIdx = 0; cswIdx < tx.GetVcswCcIn().size(); ++cswIdx)
    {
        const CTxCeasedSidechainWithdrawalInput& ccin = tx.GetVcswCcIn().at(cswIdx);

        if (!add_csw(ccin, ret_code))
        {
            LogPrintf("%s():%d Error adding csw: tx[%s], pos[%d], ret_code[%d]\n", __func__, __LINE__,
                tx_hash.ToString(), cswIdx, ret_code);
            return false;
        }
    }
    return true;
}

bool SidechainTxsCommitmentBuilder::add(const CScCertificate& cert)
{
    assert(_cmt != nullptr);

    CctpErrorCode ret_code = CctpErrorCode::OK;

    if (!add_cert(cert, ret_code))
    {
        LogPrintf("%s():%d Error adding cert[%s], ret_code[%d]\n", __func__, __LINE__,
            cert.GetHash().ToString(), ret_code);
        return false;
    }
    return true;
}

uint256 SidechainTxsCommitmentBuilder::getCommitment()
{
    assert(_cmt != nullptr);
    CctpErrorCode code;
    field_t* fe = zendoo_commitment_tree_get_commitment(const_cast<commitment_tree_t*>(_cmt), &code);
    // TODO check err code
    assert(fe != nullptr);

    wrappedFieldPtr res = {fe, CFieldPtrDeleter{}};
    CFieldElement finalTreeRoot{res};

    return finalTreeRoot.GetLegacyHashTO_BE_REMOVED();
}
#endif