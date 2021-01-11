/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file was automatically generated by chre_api_to_chpp.py
// Date: 2021-01-07 22:40:43 UTC
// Source: chre_api/include/chre_api/chre/wwan.h @ commit 70baadf2

// DO NOT modify this file directly, as those changes will be lost the next
// time the script is executed

#include "chpp/common/wwan_types.h"
#include "chpp/macros.h"
#include "chpp/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Encoding (CHRE --> CHPP) size functions

//! @return number of bytes required to represent the given
//! chreWwanCellInfoResult along with the CHPP header as
//! struct ChppWwanCellInfoResultWithHeader
static size_t chppWwanSizeOfCellInfoResultFromChre(
    const struct chreWwanCellInfoResult *cellInfoResult) {
  size_t encodedSize = sizeof(struct ChppWwanCellInfoResultWithHeader);
  encodedSize +=
      cellInfoResult->cellInfoCount * sizeof(struct ChppWwanCellInfo);
  return encodedSize;
}

// Encoding (CHRE --> CHPP) conversion functions

static void chppWwanConvertCellIdentityCdmaFromChre(
    const struct chreWwanCellIdentityCdma *in,
    struct ChppWwanCellIdentityCdma *out) {
  out->networkId = in->networkId;
  out->systemId = in->systemId;
  out->basestationId = in->basestationId;
  out->longitude = in->longitude;
  out->latitude = in->latitude;
}

static void chppWwanConvertSignalStrengthCdmaFromChre(
    const struct chreWwanSignalStrengthCdma *in,
    struct ChppWwanSignalStrengthCdma *out) {
  out->dbm = in->dbm;
  out->ecio = in->ecio;
}

static void chppWwanConvertSignalStrengthEvdoFromChre(
    const struct chreWwanSignalStrengthEvdo *in,
    struct ChppWwanSignalStrengthEvdo *out) {
  out->dbm = in->dbm;
  out->ecio = in->ecio;
  out->signalNoiseRatio = in->signalNoiseRatio;
}

static void chppWwanConvertCellInfoCdmaFromChre(
    const struct chreWwanCellInfoCdma *in, struct ChppWwanCellInfoCdma *out) {
  chppWwanConvertCellIdentityCdmaFromChre(&in->cellIdentityCdma,
                                          &out->cellIdentityCdma);
  chppWwanConvertSignalStrengthCdmaFromChre(&in->signalStrengthCdma,
                                            &out->signalStrengthCdma);
  chppWwanConvertSignalStrengthEvdoFromChre(&in->signalStrengthEvdo,
                                            &out->signalStrengthEvdo);
}

static void chppWwanConvertCellIdentityGsmFromChre(
    const struct chreWwanCellIdentityGsm *in,
    struct ChppWwanCellIdentityGsm *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->arfcn = in->arfcn;
  out->bsic = in->bsic;
  memset(&out->reserved, 0, sizeof(out->reserved));
}

static void chppWwanConvertSignalStrengthGsmFromChre(
    const struct chreWwanSignalStrengthGsm *in,
    struct ChppWwanSignalStrengthGsm *out) {
  out->signalStrength = in->signalStrength;
  out->bitErrorRate = in->bitErrorRate;
  out->timingAdvance = in->timingAdvance;
}

static void chppWwanConvertCellInfoGsmFromChre(
    const struct chreWwanCellInfoGsm *in, struct ChppWwanCellInfoGsm *out) {
  chppWwanConvertCellIdentityGsmFromChre(&in->cellIdentityGsm,
                                         &out->cellIdentityGsm);
  chppWwanConvertSignalStrengthGsmFromChre(&in->signalStrengthGsm,
                                           &out->signalStrengthGsm);
}

static void chppWwanConvertCellIdentityLteFromChre(
    const struct chreWwanCellIdentityLte *in,
    struct ChppWwanCellIdentityLte *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->ci = in->ci;
  out->pci = in->pci;
  out->tac = in->tac;
  out->earfcn = in->earfcn;
}

static void chppWwanConvertSignalStrengthLteFromChre(
    const struct chreWwanSignalStrengthLte *in,
    struct ChppWwanSignalStrengthLte *out) {
  out->signalStrength = in->signalStrength;
  out->rsrp = in->rsrp;
  out->rsrq = in->rsrq;
  out->rssnr = in->rssnr;
  out->cqi = in->cqi;
  out->timingAdvance = in->timingAdvance;
}

static void chppWwanConvertCellInfoLteFromChre(
    const struct chreWwanCellInfoLte *in, struct ChppWwanCellInfoLte *out) {
  chppWwanConvertCellIdentityLteFromChre(&in->cellIdentityLte,
                                         &out->cellIdentityLte);
  chppWwanConvertSignalStrengthLteFromChre(&in->signalStrengthLte,
                                           &out->signalStrengthLte);
}

static void chppWwanConvertCellIdentityNrFromChre(
    const struct chreWwanCellIdentityNr *in,
    struct ChppWwanCellIdentityNr *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->nci0 = in->nci0;
  out->nci1 = in->nci1;
  out->pci = in->pci;
  out->tac = in->tac;
  out->nrarfcn = in->nrarfcn;
}

static void chppWwanConvertSignalStrengthNrFromChre(
    const struct chreWwanSignalStrengthNr *in,
    struct ChppWwanSignalStrengthNr *out) {
  out->ssRsrp = in->ssRsrp;
  out->ssRsrq = in->ssRsrq;
  out->ssSinr = in->ssSinr;
  out->csiRsrp = in->csiRsrp;
  out->csiRsrq = in->csiRsrq;
  out->csiSinr = in->csiSinr;
}

static void chppWwanConvertCellInfoNrFromChre(
    const struct chreWwanCellInfoNr *in, struct ChppWwanCellInfoNr *out) {
  chppWwanConvertCellIdentityNrFromChre(&in->cellIdentityNr,
                                        &out->cellIdentityNr);
  chppWwanConvertSignalStrengthNrFromChre(&in->signalStrengthNr,
                                          &out->signalStrengthNr);
}

static void chppWwanConvertCellIdentityTdscdmaFromChre(
    const struct chreWwanCellIdentityTdscdma *in,
    struct ChppWwanCellIdentityTdscdma *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->cpid = in->cpid;
}

static void chppWwanConvertSignalStrengthTdscdmaFromChre(
    const struct chreWwanSignalStrengthTdscdma *in,
    struct ChppWwanSignalStrengthTdscdma *out) {
  out->rscp = in->rscp;
}

static void chppWwanConvertCellInfoTdscdmaFromChre(
    const struct chreWwanCellInfoTdscdma *in,
    struct ChppWwanCellInfoTdscdma *out) {
  chppWwanConvertCellIdentityTdscdmaFromChre(&in->cellIdentityTdscdma,
                                             &out->cellIdentityTdscdma);
  chppWwanConvertSignalStrengthTdscdmaFromChre(&in->signalStrengthTdscdma,
                                               &out->signalStrengthTdscdma);
}

static void chppWwanConvertCellIdentityWcdmaFromChre(
    const struct chreWwanCellIdentityWcdma *in,
    struct ChppWwanCellIdentityWcdma *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->psc = in->psc;
  out->uarfcn = in->uarfcn;
}

static void chppWwanConvertSignalStrengthWcdmaFromChre(
    const struct chreWwanSignalStrengthWcdma *in,
    struct ChppWwanSignalStrengthWcdma *out) {
  out->signalStrength = in->signalStrength;
  out->bitErrorRate = in->bitErrorRate;
}

static void chppWwanConvertCellInfoWcdmaFromChre(
    const struct chreWwanCellInfoWcdma *in, struct ChppWwanCellInfoWcdma *out) {
  chppWwanConvertCellIdentityWcdmaFromChre(&in->cellIdentityWcdma,
                                           &out->cellIdentityWcdma);
  chppWwanConvertSignalStrengthWcdmaFromChre(&in->signalStrengthWcdma,
                                             &out->signalStrengthWcdma);
}

static void chppWwanConvertCellInfoFromChre(const struct chreWwanCellInfo *in,
                                            struct ChppWwanCellInfo *out) {
  out->timeStamp = in->timeStamp;
  out->cellInfoType = in->cellInfoType;
  out->timeStampType = in->timeStampType;
  out->registered = in->registered;
  out->reserved = 0;
  memset(&out->CellInfo, 0, sizeof(out->CellInfo));
  switch (in->cellInfoType) {
    case CHRE_WWAN_CELL_INFO_TYPE_GSM:
      chppWwanConvertCellInfoGsmFromChre(&in->CellInfo.gsm, &out->CellInfo.gsm);
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_CDMA:
      chppWwanConvertCellInfoCdmaFromChre(&in->CellInfo.cdma,
                                          &out->CellInfo.cdma);
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_LTE:
      chppWwanConvertCellInfoLteFromChre(&in->CellInfo.lte, &out->CellInfo.lte);
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_WCDMA:
      chppWwanConvertCellInfoWcdmaFromChre(&in->CellInfo.wcdma,
                                           &out->CellInfo.wcdma);
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_TD_SCDMA:
      chppWwanConvertCellInfoTdscdmaFromChre(&in->CellInfo.tdscdma,
                                             &out->CellInfo.tdscdma);
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_NR:
      chppWwanConvertCellInfoNrFromChre(&in->CellInfo.nr, &out->CellInfo.nr);
      break;
    default:
      CHPP_ASSERT(false);
  }
}

static void chppWwanConvertCellInfoResultFromChre(
    const struct chreWwanCellInfoResult *in, struct ChppWwanCellInfoResult *out,
    uint8_t *payload, size_t payloadSize, uint16_t *vlaOffset) {
  out->version = CHRE_WWAN_CELL_INFO_RESULT_VERSION;
  out->errorCode = in->errorCode;
  out->cellInfoCount = in->cellInfoCount;
  out->reserved = 0;
  out->cookie = 0;

  struct ChppWwanCellInfo *cells =
      (struct ChppWwanCellInfo *)&payload[*vlaOffset];
  out->cells.length = in->cellInfoCount * sizeof(struct ChppWwanCellInfo);
  CHPP_ASSERT((size_t)(*vlaOffset + out->cells.length) <= payloadSize);
  if (out->cells.length > 0 && *vlaOffset + out->cells.length <= payloadSize) {
    for (size_t i = 0; i < in->cellInfoCount; i++) {
      chppWwanConvertCellInfoFromChre(&in->cells[i], &cells[i]);
    }
    out->cells.offset = *vlaOffset;
    *vlaOffset += out->cells.length;
  } else {
    out->cells.offset = 0;
  }
}

// Encoding (CHRE --> CHPP) top-level functions

bool chppWwanCellInfoResultFromChre(
    const struct chreWwanCellInfoResult *in,
    struct ChppWwanCellInfoResultWithHeader **out, size_t *outSize) {
  CHPP_NOT_NULL(out);
  CHPP_NOT_NULL(outSize);

  size_t payloadSize = chppWwanSizeOfCellInfoResultFromChre(in);
  *out = chppMalloc(payloadSize);
  if (*out != NULL) {
    uint8_t *payload = (uint8_t *)&(*out)->payload;
    uint16_t vlaOffset = sizeof(struct ChppWwanCellInfoResult);
    chppWwanConvertCellInfoResultFromChre(in, &(*out)->payload, payload,
                                          payloadSize, &vlaOffset);
    *outSize = payloadSize;
    return true;
  }
  return false;
}

// Decoding (CHPP --> CHRE) conversion functions

static bool chppWwanConvertCellIdentityCdmaToChre(
    const struct ChppWwanCellIdentityCdma *in,
    struct chreWwanCellIdentityCdma *out) {
  out->networkId = in->networkId;
  out->systemId = in->systemId;
  out->basestationId = in->basestationId;
  out->longitude = in->longitude;
  out->latitude = in->latitude;

  return true;
}

static bool chppWwanConvertSignalStrengthCdmaToChre(
    const struct ChppWwanSignalStrengthCdma *in,
    struct chreWwanSignalStrengthCdma *out) {
  out->dbm = in->dbm;
  out->ecio = in->ecio;

  return true;
}

static bool chppWwanConvertSignalStrengthEvdoToChre(
    const struct ChppWwanSignalStrengthEvdo *in,
    struct chreWwanSignalStrengthEvdo *out) {
  out->dbm = in->dbm;
  out->ecio = in->ecio;
  out->signalNoiseRatio = in->signalNoiseRatio;

  return true;
}

static bool chppWwanConvertCellInfoCdmaToChre(
    const struct ChppWwanCellInfoCdma *in, struct chreWwanCellInfoCdma *out) {
  if (!chppWwanConvertCellIdentityCdmaToChre(&in->cellIdentityCdma,
                                             &out->cellIdentityCdma)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthCdmaToChre(&in->signalStrengthCdma,
                                               &out->signalStrengthCdma)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthEvdoToChre(&in->signalStrengthEvdo,
                                               &out->signalStrengthEvdo)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellIdentityGsmToChre(
    const struct ChppWwanCellIdentityGsm *in,
    struct chreWwanCellIdentityGsm *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->arfcn = in->arfcn;
  out->bsic = in->bsic;
  memset(&out->reserved, 0, sizeof(out->reserved));

  return true;
}

static bool chppWwanConvertSignalStrengthGsmToChre(
    const struct ChppWwanSignalStrengthGsm *in,
    struct chreWwanSignalStrengthGsm *out) {
  out->signalStrength = in->signalStrength;
  out->bitErrorRate = in->bitErrorRate;
  out->timingAdvance = in->timingAdvance;

  return true;
}

static bool chppWwanConvertCellInfoGsmToChre(
    const struct ChppWwanCellInfoGsm *in, struct chreWwanCellInfoGsm *out) {
  if (!chppWwanConvertCellIdentityGsmToChre(&in->cellIdentityGsm,
                                            &out->cellIdentityGsm)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthGsmToChre(&in->signalStrengthGsm,
                                              &out->signalStrengthGsm)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellIdentityLteToChre(
    const struct ChppWwanCellIdentityLte *in,
    struct chreWwanCellIdentityLte *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->ci = in->ci;
  out->pci = in->pci;
  out->tac = in->tac;
  out->earfcn = in->earfcn;

  return true;
}

static bool chppWwanConvertSignalStrengthLteToChre(
    const struct ChppWwanSignalStrengthLte *in,
    struct chreWwanSignalStrengthLte *out) {
  out->signalStrength = in->signalStrength;
  out->rsrp = in->rsrp;
  out->rsrq = in->rsrq;
  out->rssnr = in->rssnr;
  out->cqi = in->cqi;
  out->timingAdvance = in->timingAdvance;

  return true;
}

static bool chppWwanConvertCellInfoLteToChre(
    const struct ChppWwanCellInfoLte *in, struct chreWwanCellInfoLte *out) {
  if (!chppWwanConvertCellIdentityLteToChre(&in->cellIdentityLte,
                                            &out->cellIdentityLte)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthLteToChre(&in->signalStrengthLte,
                                              &out->signalStrengthLte)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellIdentityNrToChre(
    const struct ChppWwanCellIdentityNr *in,
    struct chreWwanCellIdentityNr *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->nci0 = in->nci0;
  out->nci1 = in->nci1;
  out->pci = in->pci;
  out->tac = in->tac;
  out->nrarfcn = in->nrarfcn;

  return true;
}

static bool chppWwanConvertSignalStrengthNrToChre(
    const struct ChppWwanSignalStrengthNr *in,
    struct chreWwanSignalStrengthNr *out) {
  out->ssRsrp = in->ssRsrp;
  out->ssRsrq = in->ssRsrq;
  out->ssSinr = in->ssSinr;
  out->csiRsrp = in->csiRsrp;
  out->csiRsrq = in->csiRsrq;
  out->csiSinr = in->csiSinr;

  return true;
}

static bool chppWwanConvertCellInfoNrToChre(const struct ChppWwanCellInfoNr *in,
                                            struct chreWwanCellInfoNr *out) {
  if (!chppWwanConvertCellIdentityNrToChre(&in->cellIdentityNr,
                                           &out->cellIdentityNr)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthNrToChre(&in->signalStrengthNr,
                                             &out->signalStrengthNr)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellIdentityTdscdmaToChre(
    const struct ChppWwanCellIdentityTdscdma *in,
    struct chreWwanCellIdentityTdscdma *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->cpid = in->cpid;

  return true;
}

static bool chppWwanConvertSignalStrengthTdscdmaToChre(
    const struct ChppWwanSignalStrengthTdscdma *in,
    struct chreWwanSignalStrengthTdscdma *out) {
  out->rscp = in->rscp;

  return true;
}

static bool chppWwanConvertCellInfoTdscdmaToChre(
    const struct ChppWwanCellInfoTdscdma *in,
    struct chreWwanCellInfoTdscdma *out) {
  if (!chppWwanConvertCellIdentityTdscdmaToChre(&in->cellIdentityTdscdma,
                                                &out->cellIdentityTdscdma)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthTdscdmaToChre(
          &in->signalStrengthTdscdma, &out->signalStrengthTdscdma)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellIdentityWcdmaToChre(
    const struct ChppWwanCellIdentityWcdma *in,
    struct chreWwanCellIdentityWcdma *out) {
  out->mcc = in->mcc;
  out->mnc = in->mnc;
  out->lac = in->lac;
  out->cid = in->cid;
  out->psc = in->psc;
  out->uarfcn = in->uarfcn;

  return true;
}

static bool chppWwanConvertSignalStrengthWcdmaToChre(
    const struct ChppWwanSignalStrengthWcdma *in,
    struct chreWwanSignalStrengthWcdma *out) {
  out->signalStrength = in->signalStrength;
  out->bitErrorRate = in->bitErrorRate;

  return true;
}

static bool chppWwanConvertCellInfoWcdmaToChre(
    const struct ChppWwanCellInfoWcdma *in, struct chreWwanCellInfoWcdma *out) {
  if (!chppWwanConvertCellIdentityWcdmaToChre(&in->cellIdentityWcdma,
                                              &out->cellIdentityWcdma)) {
    return false;
  }
  if (!chppWwanConvertSignalStrengthWcdmaToChre(&in->signalStrengthWcdma,
                                                &out->signalStrengthWcdma)) {
    return false;
  }

  return true;
}

static bool chppWwanConvertCellInfoToChre(const struct ChppWwanCellInfo *in,
                                          struct chreWwanCellInfo *out) {
  out->timeStamp = in->timeStamp;
  out->cellInfoType = in->cellInfoType;
  out->timeStampType = in->timeStampType;
  out->registered = in->registered;
  out->reserved = 0;
  memset(&out->CellInfo, 0, sizeof(out->CellInfo));
  switch (in->cellInfoType) {
    case CHRE_WWAN_CELL_INFO_TYPE_GSM:
      if (!chppWwanConvertCellInfoGsmToChre(&in->CellInfo.gsm,
                                            &out->CellInfo.gsm)) {
        return false;
      }
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_CDMA:
      if (!chppWwanConvertCellInfoCdmaToChre(&in->CellInfo.cdma,
                                             &out->CellInfo.cdma)) {
        return false;
      }
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_LTE:
      if (!chppWwanConvertCellInfoLteToChre(&in->CellInfo.lte,
                                            &out->CellInfo.lte)) {
        return false;
      }
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_WCDMA:
      if (!chppWwanConvertCellInfoWcdmaToChre(&in->CellInfo.wcdma,
                                              &out->CellInfo.wcdma)) {
        return false;
      }
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_TD_SCDMA:
      if (!chppWwanConvertCellInfoTdscdmaToChre(&in->CellInfo.tdscdma,
                                                &out->CellInfo.tdscdma)) {
        return false;
      }
      break;
    case CHRE_WWAN_CELL_INFO_TYPE_NR:
      if (!chppWwanConvertCellInfoNrToChre(&in->CellInfo.nr,
                                           &out->CellInfo.nr)) {
        return false;
      }
      break;
    default:
      CHPP_ASSERT(false);
  }

  return true;
}

static bool chppWwanConvertCellInfoResultToChre(
    const struct ChppWwanCellInfoResult *in, struct chreWwanCellInfoResult *out,
    size_t inSize) {
  out->version = CHRE_WWAN_CELL_INFO_RESULT_VERSION;
  out->errorCode = in->errorCode;
  out->cellInfoCount = in->cellInfoCount;
  out->reserved = 0;
  out->cookie = 0;

  if (in->cells.length == 0) {
    out->cells = NULL;
  } else {
    if (in->cells.offset + in->cells.length > inSize ||
        in->cells.length !=
            in->cellInfoCount * sizeof(struct chreWwanCellInfo)) {
      return false;
    }

    const struct ChppWwanCellInfo *cellsIn = (const struct ChppWwanCellInfo *)&(
        (const uint8_t *)in)[in->cells.offset];

    struct chreWwanCellInfo *cellsOut =
        chppMalloc(in->cellInfoCount * sizeof(struct chreWwanCellInfo));
    if (cellsOut == NULL) {
      return false;
    }

    for (size_t i = 0; i < in->cellInfoCount; i++) {
      if (!chppWwanConvertCellInfoToChre(&cellsIn[i], &cellsOut[i])) {
        return false;
      }
    }
    out->cells = cellsOut;
  }

  return true;
}

// Decoding (CHPP --> CHRE) top-level functions

struct chreWwanCellInfoResult *chppWwanCellInfoResultToChre(
    const struct ChppWwanCellInfoResult *in, size_t inSize) {
  struct chreWwanCellInfoResult *out = NULL;

  if (inSize >= sizeof(struct ChppWwanCellInfoResult)) {
    out = chppMalloc(sizeof(struct chreWwanCellInfoResult));
    if (out != NULL) {
      if (!chppWwanConvertCellInfoResultToChre(in, out, inSize)) {
        CHPP_FREE_AND_NULLIFY(out);
      }
    }
  }

  return out;
}
