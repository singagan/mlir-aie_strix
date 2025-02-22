//===- test_library.h -------------------------------------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// (c) Copyright 2021 Xilinx Inc.
//
//===----------------------------------------------------------------------===//
#ifndef AIE_TEST_LIBRARY_H
#define AIE_TEST_LIBRARY_H
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <xaiengine.h>

extern "C" {

#define mlir_aie_check(s, r, v, errors)                                        \
  if (r != v) {                                                                \
    printf("ERROR %s: %s expected %d, but was %d!\n", s, #r, v, r);            \
    errors++;                                                                  \
  }
#define mlir_aie_check_float(s, r, v, errors)                                  \
  if (r != v) {                                                                \
    printf("ERROR %s: %s expected %f, but was %f!\n", s, #r, v, r);            \
    errors++;                                                                  \
  }

/*
 ******************************************************************************
 * LIBXAIENGIENV2
 ******************************************************************************
 */
#ifdef LIBXAIENGINEV2

#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_NUM_ROWS 9
#define XAIE_NUM_COLS 50
#define XAIE_COL_SHIFT 23
#define XAIE_ROW_SHIFT 18
#define XAIE_SHIM_ROW 0
#define XAIE_RES_TILE_ROW_START 0
#define XAIE_RES_TILE_NUM_ROWS 0
#define XAIE_AIE_TILE_ROW_START 1
#define XAIE_AIE_TILE_NUM_ROWS 8

// XAie_SetupConfig(ConfigPtr, XAIE_DEV_GEN_AIE, XAIE_BASE_ADDR,
//        XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
//        XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
//        XAIE_RES_TILE_ROW_START, XAIE_RES_TILE_NUM_ROWS,
//        XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);
/*
XAie_Config ConfigPtr = {XAIE_DEV_GEN_AIE, XAIE_BASE_ADDR,
        XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
        XAIE_NUM_ROWS, XAIE_NUM_COLS, XAIE_SHIM_ROW,
        XAIE_RES_TILE_ROW_START, XAIE_RES_TILE_NUM_ROWS,
        XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS, {0}, };
//XAie_InstDeclare(DevInst, &ConfigPtr);
XAie_DevInst DevInst = { 0 };
*/

struct aie_libxaie_ctx_t {
  XAie_Config AieConfigPtr;
  XAie_DevInst DevInst;
  /*
    XAieGbl_Config *AieConfigPtr;
    XAieGbl AieInst;
    XAieGbl_HwCfg AieConfig;
    XAieGbl_Tile TileInst[XAIE_NUM_COLS][XAIE_NUM_ROWS+1];
    XAieDma_Tile TileDMAInst[XAIE_NUM_COLS][XAIE_NUM_ROWS+1];
  */
  XAie_MemInst **buffers;
};

/*
 ******************************************************************************
 * LIBXAIENGIENV1
 ******************************************************************************
 */
#else

#define XAIE_NUM_ROWS 8
#define XAIE_NUM_COLS 50
#define XAIE_ADDR_ARRAY_OFF 0x800

#define MODE_CORE 0
#define MODE_PL 1
#define MODE_MEM 2

//#define HIGH_ADDR(addr)	((addr & 0xffffffff00000000) >> 32)
//#define LOW_ADDR(addr)	(addr & 0x00000000ffffffff)

//#define MLIR_STACK_OFFSET 4096

struct aie_libxaie_ctx_t {
  XAieGbl_Config *AieConfigPtr;
  XAieGbl AieInst;
  XAieGbl_HwCfg AieConfig;
  XAieGbl_Tile TileInst[XAIE_NUM_COLS][XAIE_NUM_ROWS + 1];
  XAieDma_Tile TileDMAInst[XAIE_NUM_COLS][XAIE_NUM_ROWS + 1];
};


// class for using events and PF cpounters
class EventMonitor {
public:
  EventMonitor(struct XAieGbl_Tile *_tilePtr, u32 _pfc, u32 _startE, u32 _endE,
           u32 _resetE, u8 _mode) {
    tilePtr = _tilePtr;
    pfc = _pfc;
    mode = _mode; // 0: Core, 1: PL, 2, Mem
    if (mode == MODE_CORE) {
      XAieTileCore_PerfCounterControl(tilePtr, pfc, _startE, _endE, _resetE);
    } else if (mode == MODE_PL) {
      XAieTilePl_PerfCounterControl(tilePtr, pfc, _startE, _endE, _resetE);
    } else {
      XAieTileMem_PerfCounterControl(tilePtr, pfc, _startE, _endE, _resetE);
    }
  }
  void set() {
    if (mode == MODE_CORE) {
      start = XAieTileCore_PerfCounterGet(tilePtr, pfc);
    } else if (mode == MODE_PL) {
      start = XAieTilePl_PerfCounterGet(tilePtr, pfc);
    } else {
      start = XAieTileMem_PerfCounterGet(tilePtr, pfc);
    }
  }
  u32 read() {
    if (mode == MODE_CORE) {
      return XAieTileCore_PerfCounterGet(tilePtr, pfc);
    } else if (mode == MODE_PL) {
      return XAieTilePl_PerfCounterGet(tilePtr, pfc);
    } else {
      return XAieTileMem_PerfCounterGet(tilePtr, pfc);
    }
  }
  u32 diff() {
    u32 end;
    if (mode == MODE_CORE) {
      end = XAieTileCore_PerfCounterGet(tilePtr, pfc);
    } else if (mode == MODE_PL) {
      end = XAieTilePl_PerfCounterGet(tilePtr, pfc);
    } else {
      end = XAieTileMem_PerfCounterGet(tilePtr, pfc);
    }
    if (end < start) {
      printf("WARNING: EventMonitor: performance counter wrapped!\n");
      return 0; // TODO: fix this
    } else {
      return end - start;
    }
  }

private:
  u32 start;
  u32 pfc;
  u8 mode;
  struct XAieGbl_Tile *tilePtr;
};

void computeStats(u32 performance_counter[], int n);

#endif

aie_libxaie_ctx_t *mlir_aie_init_libxaie();
void mlir_aie_deinit_libxaie(aie_libxaie_ctx_t *);

int mlir_aie_init_device(aie_libxaie_ctx_t *ctx);

int mlir_aie_acquire_lock(aie_libxaie_ctx_t *ctx, int col, int row, int lockid,
                          int lockval, int timeout);
int mlir_aie_release_lock(aie_libxaie_ctx_t *ctx, int col, int row, int lockid,
                          int lockval, int timeout);
u32 mlir_aie_read32(aie_libxaie_ctx_t *ctx, u64 addr);
void mlir_aie_write32(aie_libxaie_ctx_t *ctx, u64 addr, u32 val);
u32 mlir_aie_data_mem_rd_word(aie_libxaie_ctx_t *ctx, int col, int row,
                              u64 addr);
void mlir_aie_data_mem_wr_word(aie_libxaie_ctx_t *ctx, int col, int row,
                               u64 addr, u32 data);

u64 mlir_aie_get_tile_addr(aie_libxaie_ctx_t *ctx, int col, int row);

/// Dump the contents of the memory associated with the given tile.
void mlir_aie_dump_tile_memory(aie_libxaie_ctx_t *ctx, int col, int row);

/// Clear the contents of the memory associated with the given tile.
void mlir_aie_clear_tile_memory(aie_libxaie_ctx_t *ctx, int col, int row);

/// Print the status of a dma represented by the given tile.
void mlir_aie_print_dma_status(aie_libxaie_ctx_t *ctx, int col, int row);

/// Print the status of a shimdma represented by the given location (row = 0).
void mlir_aie_print_shimdma_status(aie_libxaie_ctx_t *ctx, int col, int row);

/// Print the status of a core represented by the given tile.
void mlir_aie_print_tile_status(aie_libxaie_ctx_t *ctx, int col, int row);

/// Zero out the program and configuration memory of the tile.
void mlir_aie_clear_config(aie_libxaie_ctx_t *ctx, int col, int row);

/// Zero out the configuration memory of the shim tile.
void mlir_aie_clear_shim_config(aie_libxaie_ctx_t *ctx, int col, int row);

void mlir_aie_init_mems(aie_libxaie_ctx_t *ctx, int numBufs);
int *mlir_aie_mem_alloc(aie_libxaie_ctx_t *ctx, int bufIdx, u64 addr, int size);
void mlir_aie_sync_mem_cpu(aie_libxaie_ctx_t *ctx, int bufIdx);
void mlir_aie_sync_mem_dev(aie_libxaie_ctx_t *ctx, int bufIdx);

} // extern "C"

#endif
