# HardwareTokenParserTests failing on invalid input

## Summary
Token parsing functions throw exceptions for invalid numeric input instead of returning gracefully and/or setting Status. This breaks the new edge-case unit tests and indicates crash potential for malformed programs.

## Failed Tests
- HardwareTokenParserTests
  - InvalidIntegerCases/InvalidIntegerNoThrowTest.*
  - InvalidSignedByteCases/InvalidSignedByteNoThrowTest.*

## Evidence
- TokenParser::integer throws std::invalid_argument/std::out_of_range from std::stoi on invalid input.
- TokenParser::get_signed_byte throws std::invalid_argument/std::out_of_range and leaves Status::p_err false.

## Suspected Areas
- src/core/hardware/token_parser.cpp
  - TokenParser::integer
  - TokenParser::get_signed_byte

## Suggested Fix Direction
- Catch std::invalid_argument/std::out_of_range in TokenParser::integer and TokenParser::get_signed_byte.
- Set Status::p_err and return default values on error.

## Impact
Malformed user programs can crash parsing instead of producing error messages.
