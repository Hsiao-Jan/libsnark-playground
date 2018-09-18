#include <stdlib.h>
#include <iostream>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"
#include "libsnark/gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp"

using namespace libsnark;
using namespace std;

int main() {
    // Initialize the curve parameters

    default_r1cs_ppzksnark_pp::init_public_params();

    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;

    protoboard<FieldT> pb;

    digest_variable<FieldT> left(pb, SHA256_digest_size, "left");
    digest_variable<FieldT> right(pb, SHA256_digest_size, "right");
    digest_variable<FieldT> output(pb, SHA256_digest_size, "output");

    sha256_two_to_one_hash_gadget<FieldT> f(pb, left, right, output, "f");
    f.generate_r1cs_constraints();
    printf("Number of constraints for sha256_two_to_one_hash_gadget: %zu\n", pb.num_constraints());

    // Trusted setup
    const r1cs_constraint_system<FieldT> constraint_system = pb.get_constraint_system();

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(
            constraint_system);

    // Add witness values

    // Empty string
    const libff::bit_vector left_bv = libff::int_list_to_bits({0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, 32);
    const libff::bit_vector right_bv = libff::int_list_to_bits({0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, 32);
    const libff::bit_vector hash_bv = libff::int_list_to_bits({0xda5698be, 0x17b9b469, 0x62335799, 0x779fbeca, 0x8ce5d491, 0xc0d26243, 0xbafef9ea, 0x1837a9d8}, 32);

    // concatenation of bytes:
    // [197, 215, 20, 132, 248, 207, 155, 244, 183, 111, 71, 144, 71, 48, 128, 75, 158, 50, 37, 169, 241, 51, 181, 222, 161, 104, 244, 226, 133, 31, 7, 47]
    // and
    // [204, 0, 252, 170, 124, 166, 32, 97, 113, 122, 72, 229, 46, 41, 163, 250, 55, 154, 149, 63, 170, 104, 147, 227, 46, 197, 162, 123, 148, 94, 96, 95]
    /*const libff::bit_vector left_bv = libff::int_list_to_bits({0x426bc2d8, 0x4dc86782, 0x81e8957a, 0x409ec148, 0xe6cffbe8, 0xafe6ba4f, 0x9c6f1978, 0xdd7af7e9}, 32);
    const libff::bit_vector right_bv = libff::int_list_to_bits({0x038cce42, 0xabd366b8, 0x3ede7e00, 0x9130de53, 0x72cdf73d, 0xee825114, 0x8cb48d1b, 0x9af68ad0}, 32);
    const libff::bit_vector hash_bv = libff::int_list_to_bits({0xeffd0b7f, 0x1ccba116, 0x2ee816f7, 0x31c62b48, 0x59305141, 0x990e5c0a, 0xce40d33d, 0x0b1167d1}, 32);*/

    left.generate_r1cs_witness(left_bv);
    right.generate_r1cs_witness(right_bv);

    f.generate_r1cs_witness();
    output.generate_r1cs_witness(hash_bv);

    cout << "Satisfied status: " << pb.is_satisfied() << endl;

    // Create proof
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof1 = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, pb.primary_input(), pb.auxiliary_input());

    // Verify proof
    bool verified1 = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, pb.primary_input(), proof1);

    cout << "Verfied: " << verified1 << endl;

    return 0;
}