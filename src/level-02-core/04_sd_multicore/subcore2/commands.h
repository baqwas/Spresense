/**
 * @file        commands.h
 * @author      Matha Goram
 * @brief       IPC Command Definitions and Shared Data Structures.
 * @details     This header defines the communication protocol between the 
 * Main Core (ID 0) and SubCores (1-5). It establishes a 
 * standardized message format for the MP Mailbox system, 
 * enabling zero-copy memory sharing across the CXD5602 
 * asymmetric multiprocessing (AMP) architecture.
 * * @note        This file must be included in both the Main Core and 
 * relevant SubCore projects to ensure memory alignment.
 * * @target      Sony Spresense (CXD5602)
 * @version     1.0.0
 * @date        2026-01-15
 * * @copyright   Copyright (c) 2026 ParkCircus Productions
 * * @license   MIT
 * * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef COMMANDS_H
#define COMMANDS_H

enum SDCommand {
  CMD_IDLE = 0,
  CMD_WRITE_LOG,
  CMD_DELETE_ALL
};

struct SDMessage {
  SDCommand command;
  char payload[32]; // Filename or data string
};

#endif