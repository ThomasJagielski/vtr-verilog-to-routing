/*
 * Copyright 2023 CAS—Atlantic (University of New Brunswick, CASA)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
 
`timescale 1ps/1ps

`define MEM_MAXADDR PPP
`define MEM_MAXDATA 36

// depth and data may need to be splited
module dual_port_ram(clk, we1, we2, addr1, addr2, data1, data2, out1, out2);
    parameter ADDR_WIDTH = `MEM_MAXADDR;
    parameter DATA_WIDTH = 1;

    input clk;
    input we1, we2;
    input [ADDR_WIDTH-1:0] addr1, addr2;
    input [DATA_WIDTH-1:0] data1, data2;

    output reg [DATA_WIDTH-1:0] out1, out2;

    genvar i;
	generate 
		// split in depth
		if (ADDR_WIDTH > `MEM_MAXADDR)
		begin
			
            wire [ADDR_WIDTH-2:0] new_addr1 = addr1[ADDR_WIDTH-2:0];
            wire [ADDR_WIDTH-2:0] new_addr2 = addr2[ADDR_WIDTH-2:0];

            wire [DATA_WIDTH-1:0] out1_h, out1_l;
            wire [DATA_WIDTH-1:0] out2_h, out2_l;


            defparam uut_h.ADDR_WIDTH = ADDR_WIDTH-1;
            defparam uut_h.DATA_WIDTH = DATA_WIDTH;
            dual_port_ram uut_h (
                .clk(clk), 
                .we1(we1), 
                .we2(we2), 
                .addr1(new_addr1), 
                .addr2(new_addr2), 
                .data1(data1), 
                .data2(data2), 
                .out1(out1_h),
                .out2(out2_h)
            );

            defparam uut_l.ADDR_WIDTH = ADDR_WIDTH-1;
            defparam uut_l.DATA_WIDTH = DATA_WIDTH;
            dual_port_ram uut_l (
                .clk(clk), 
                .we1(we1), 
                .we2(we2), 
                .addr1(new_addr1), 
                .addr2(new_addr2), 
                .data1(data1), 
                .data2(data2), 
                .out1(out1_l),
                .out2(out2_l)
            );

            reg additional_bit;
            always @(posedge clk) additional_bit <= addr[ADDR_WIDTH-1];
            assign out1 = (additional_bit) ? out1_h : out1_l;
            assign out2 = (additional_bit) ? out2_h : out2_l;

        end	else begin
            for (i = 0; i < DATA_WIDTH; i = i + 1) begin:single_bit_data
                dualPortRam uut (
                    .clk(clk), 
                    .we1(we1), 
                    .we2(we2), 
                    .addr1({ {{`MEM_MAXADDR-ADDR_WIDTH}{1'bx}}, addr1[ADDR_WIDTH-1:0] }), 
                    .addr2({ {{`MEM_MAXADDR-ADDR_WIDTH}{1'bx}}, addr2[ADDR_WIDTH-1:0] }), 
                    .data1(data1[i]), 
                    .data2(data2[i]), 
                    .out1(out1[i]),
                    .out2(out2[i])
                );
            end
        end
    endgenerate
    
endmodule



(* blackbox *)
module dualPortRam(clk, data2, data1, addr2, addr1, we2, we1, out2, out1);
    localparam ADDR_WIDTH = `MEM_MAXADDR;
    localparam DATA_WIDTH = 1;

    input clk;
    input we1, we2;
    input [ADDR_WIDTH-1:0] addr1, addr2;
    input data1, data2;

    output reg out1, out2;
    /*
    reg [DATA_WIDTH-1:0] RAM [(1<<ADDR_WIDTH)-1:0];

    always @(posedge clk)
    begin
        if (we1)
                RAM[addr1] <= data1;
        if (we2)
                RAM[addr2] <= data2;
                
        out1 <= RAM[addr1];
        out2 <= RAM[addr2];
    end
    */
endmodule

