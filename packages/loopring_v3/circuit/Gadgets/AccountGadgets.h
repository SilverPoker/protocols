// SPDX-License-Identifier: Apache-2.0
// Copyright 2017 Loopring Technology Limited.
#ifndef _ACCOUNTGADGETS_H_
#define _ACCOUNTGADGETS_H_

#include "../Utils/Constants.h"
#include "../Utils/Data.h"

#include "MerkleTree.h"

#include "ethsnarks.hpp"
#include "utils.hpp"
#include "gadgets/merkle_tree.hpp"
#include "gadgets/poseidon.hpp"

using namespace ethsnarks;

namespace Loopring {

struct AccountState {
  VariableT owner;
  VariableT publicKeyX;
  VariableT publicKeyY;
  VariableT nonce;
  VariableT balancesRoot;
};

static void printAccount(const ProtoboardT &pb, const AccountState &state) {
  std::cout << "- owner: " << pb.val(state.owner) << std::endl;
  std::cout << "- publicKeyX: " << pb.val(state.publicKeyX) << std::endl;
  std::cout << "- publicKeyY: " << pb.val(state.publicKeyY) << std::endl;
  std::cout << "- nonce: " << pb.val(state.nonce) << std::endl;
  std::cout << "- balancesRoot: " << pb.val(state.balancesRoot) << std::endl;
}

class AccountGadget : public GadgetT {
public:
  VariableT owner;
  const jubjub::VariablePointT publicKey;
  VariableT nonce;
  VariableT balancesRoot;

  AccountGadget(ProtoboardT &pb, const std::string &_prefix)
      : GadgetT(pb, _prefix),

        owner(make_variable(pb, FMT(_prefix, ".owner"))),
        publicKey(pb, FMT(_prefix, ".publicKey")),
        nonce(make_variable(pb, FMT(_prefix, ".nonce"))),
        balancesRoot(make_variable(pb, FMT(_prefix, ".balancesRoot"))) {}

  void generate_r1cs_witness(const Account &account) {
    pb.val(owner) = account.owner;
    pb.val(publicKey.x) = account.publicKey.x;
    pb.val(publicKey.y) = account.publicKey.y;
    pb.val(nonce) = account.nonce;
    pb.val(balancesRoot) = account.balancesRoot;
  }
};

class UpdateAccountGadget : public GadgetT {
public:
  HashAccountLeaf leafHashBefore;
  HashAccountLeaf leafHashAfter;

  AccountState leafBefore;
  AccountState leafAfter;

  const VariableArrayT proof;
  VerifyTreeRoot rootBeforeVerifier;
  UpdateTreeRoot rootAfter;

  UpdateAccountGadget(ProtoboardT &pb, const VariableT &_rootBefore,
                      const VariableArrayT &_addressBits,
                      const AccountState &_leafBefore,
                      const AccountState &_leafAfter,
                      const std::string &_prefix)
      : GadgetT(pb, _prefix),

        leafBefore(_leafBefore), leafAfter(_leafAfter),

        leafHashBefore(pb,
                       var_array({_leafBefore.owner, _leafBefore.publicKeyX,
                                  _leafBefore.publicKeyY, _leafBefore.nonce,
                                  _leafBefore.balancesRoot}),
                       FMT(_prefix, ".leafHashBefore")),
        leafHashAfter(pb,
                      var_array({_leafAfter.owner, _leafAfter.publicKeyX,
                                 _leafAfter.publicKeyY, _leafAfter.nonce,
                                 _leafAfter.balancesRoot}),
                      FMT(_prefix, ".leafHashAfter")),

        proof(make_var_array(pb, TREE_DEPTH_ACCOUNTS * 3,
                             FMT(_prefix, ".proof"))),
        rootBeforeVerifier(pb, TREE_DEPTH_ACCOUNTS, _addressBits,
                           leafHashBefore.result(), _rootBefore, proof,
                           FMT(_prefix, ".pathBefore")),
        rootAfter(pb, TREE_DEPTH_ACCOUNTS, _addressBits, leafHashAfter.result(),
                  proof, FMT(_prefix, ".pathAfter")) {}

  void generate_r1cs_witness(const AccountUpdate &update) {
    leafHashBefore.generate_r1cs_witness();
    leafHashAfter.generate_r1cs_witness();

    proof.fill_with_field_elements(pb, update.proof.data);
    rootBeforeVerifier.generate_r1cs_witness();
    rootAfter.generate_r1cs_witness();

    // ASSERT(pb.val(rootBeforeVerifier.m_expected_root) == update.rootBefore,
    // annotation__prefix);
    if (pb.val(rootAfter.result()) != update.rootAfter) {
      std::cout << "leafBefore:" << std::endl;
      printAccount(pb, leafBefore);
      std::cout << "leafAfter:" << std::endl;
      printAccount(pb, leafAfter);
      ASSERT(pb.val(rootAfter.result()) == update.rootAfter,
             annotation__prefix);
    }
  }

  void generate_r1cs_constraints() {
    leafHashBefore.generate_r1cs_constraints();
    leafHashAfter.generate_r1cs_constraints();

    rootBeforeVerifier.generate_r1cs_constraints();
    rootAfter.generate_r1cs_constraints();
  }

  const VariableT &result() const { return rootAfter.result(); }
};

struct BalanceState {
  VariableT balance;
  VariableT storage;
};

static void printBalance(const ProtoboardT &pb, const BalanceState &state) {
  std::cout << "- balance: " << pb.val(state.balance) << std::endl;
  std::cout << "- storage: " << pb.val(state.storage) << std::endl;
}

class BalanceGadget : public GadgetT {
public:
  VariableT balance;
  VariableT storage;

  BalanceGadget(ProtoboardT &pb, const std::string &_prefix)
      : GadgetT(pb, _prefix),

        balance(make_variable(pb, FMT(_prefix, ".balance"))),
        storage(make_variable(pb, FMT(_prefix, ".storage"))) {}

  void generate_r1cs_witness(const BalanceLeaf &balanceLeaf) {
    pb.val(balance) = balanceLeaf.balance;
    pb.val(storage) = balanceLeaf.storageRoot;
  }
};

class UpdateBalanceGadget : public GadgetT {
public:
  HashBalanceLeaf leafHashBefore;
  HashBalanceLeaf leafHashAfter;

  BalanceState leafBefore;
  BalanceState leafAfter;

  const VariableArrayT proof;
  VerifyTreeRoot rootBeforeVerifier;
  UpdateTreeRoot rootAfter;

  UpdateBalanceGadget(ProtoboardT &pb, const VariableT &_rootBefore,
                      const VariableArrayT &_tokenID,
                      const BalanceState _leafBefore,
                      const BalanceState _leafAfter, const std::string &_prefix)
      : GadgetT(pb, _prefix),

        leafBefore(_leafBefore), leafAfter(_leafAfter),

        leafHashBefore(pb,
                       var_array({_leafBefore.balance, _leafBefore.storage}),
                       FMT(_prefix, ".leafHashBefore")),
        leafHashAfter(pb, var_array({_leafAfter.balance, _leafAfter.storage}),
                      FMT(_prefix, ".leafHashAfter")),

        proof(
            make_var_array(pb, TREE_DEPTH_TOKENS * 3, FMT(_prefix, ".proof"))),
        rootBeforeVerifier(pb, TREE_DEPTH_TOKENS, _tokenID,
                           leafHashBefore.result(), _rootBefore, proof,
                           FMT(_prefix, ".pathBefore")),
        rootAfter(pb, TREE_DEPTH_TOKENS, _tokenID, leafHashAfter.result(),
                  proof, FMT(_prefix, ".pathAfter")) {}

  void generate_r1cs_witness(const BalanceUpdate &update) {
    leafHashBefore.generate_r1cs_witness();
    leafHashAfter.generate_r1cs_witness();

    proof.fill_with_field_elements(pb, update.proof.data);
    rootBeforeVerifier.generate_r1cs_witness();
    rootAfter.generate_r1cs_witness();

    // ASSERT(pb.val(rootBeforeVerifier.m_expected_root) == update.rootBefore,
    // annotation__prefix);
    if (pb.val(rootAfter.result()) != update.rootAfter) {
      std::cout << "leafBefore:" << std::endl;
      printBalance(pb, leafBefore);
      std::cout << "leafAfter:" << std::endl;
      printBalance(pb, leafAfter);
      ASSERT(pb.val(rootAfter.result()) == update.rootAfter,
             annotation__prefix);
    }
  }

  void generate_r1cs_constraints() {
    leafHashBefore.generate_r1cs_constraints();
    leafHashAfter.generate_r1cs_constraints();

    rootBeforeVerifier.generate_r1cs_constraints();
    rootAfter.generate_r1cs_constraints();
  }

  const VariableT &result() const { return rootAfter.result(); }
};

// Calculcates the state of a user's open position
class DynamicBalanceGadget : public DynamicVariableGadget {
public:
  DynamicBalanceGadget(ProtoboardT &pb, const Constants &_balance,
                       const VariableT &balance, const std::string &_prefix)
      : DynamicVariableGadget(pb, _prefix) {
    add(balance);
    allowGeneratingWitness = false;
  }

  DynamicBalanceGadget(ProtoboardT &pb, const Constants &_constants,
                       const BalanceGadget &_balance,
                       const std::string &_prefix)
      : DynamicBalanceGadget(pb, _constants, _balance.balance, _prefix) {}

  void generate_r1cs_witness() {}

  void generate_r1cs_constraints() {}

  const VariableT &balance() const { return back(); }
};

} // namespace Loopring

#endif
