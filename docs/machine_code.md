
# Command List
- ADD, CALL, COPY, DEC, DIG, INC, JNF, JNZ, JMP, LT, LOAD, MOVE, NOP, POP, PUSH, RT, RET, SUB
- Total: 17

# Instruction Encoding (8 bit words)
- Supports 32 operations: 5 bits to encode instruction
- Additional 3 bits for extra info

# NOP Instruction (8 bits)
- Instruction: 0b00000

# Movement Instruction (8 bits)
- Instruction: 0b00001

# Load Constant Instruction (40 bits)
- Instruction: 0b00010
- Register A:  0b000
- Register B:  0b001
- Constant: next 32 bits (4 instructions) (little endian - least significant first)

# Two Register Instruction (8 bits)
- COPY (0b00011) / ADD (0b00100) / SUB (0b00101)
- Register A -> A: 0b000
- Register A -> B: 0b001
- Register B -> A: 0b010
- Register B -> B: 0b011

# Single Register Instruction (8 bits)
- INC (0b00110) / DEC (0b00111) / PUSH (0b01000) / POP (0b01001)
- Register A: 0b000
- Register B: 0b001

# Jump Instruction (24 bits)
- JMP (0b01010) / JNZ (0b01011) / CALL (0b01100) / RET (0b01101) / JNF (0b01110)
- Additional: 0b000
- Address: next 16 bits (2 instructions)

# Turn Instructions
- Left (0b01111) / Right (0b10000)

# Dig Instruction
- Instruction: 0b10001