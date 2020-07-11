/*

  Copyright 2017 Loopring Project Ltd (Loopring Foundation).

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
pragma solidity ^0.6.10;

import "../lib/OwnerManagable.sol";
import "../lib/SignatureUtil.sol";
import "../thirdparty/BytesUtil.sol";
import "../thirdparty/ERC1271.sol";


/// @title OfficialGuardian
/// @author Freeman Zhong - <kongliang@loopring.org>
contract OfficialGuardian is OwnerManagable, ERC1271
{
    using SignatureUtil for bytes;

    function isValidSignature(
        bytes memory _data,
        bytes memory _signature
        )
        public
        view
        override
        returns (bytes4)
    {
        address signer = _data.recoverECDSASigner(_signature);
        return isManager(signer) ?  MAGICVALUE : bytes4(0);
    }
}