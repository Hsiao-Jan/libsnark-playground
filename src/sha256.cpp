#include <stdlib.h>
#include <iostream>
#include <chrono>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"
#include "libsnark/gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp"

using namespace libsnark;
using namespace std;

typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;

template<typename ppT, typename FieldT>
bool verify_proof(r1cs_ppzksnark_verification_key<ppT> verification_key, r1cs_primary_input<FieldT> primary_input,
        r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof) {
    return r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(verification_key, primary_input, proof);
}

r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> setup_gadget(protoboard<FieldT> &pb, block_variable<FieldT> *&inp, digest_variable<FieldT> *&out, sha256_two_to_one_hash_gadget<FieldT> *&g) {
    inp = new block_variable<FieldT>(pb, SHA256_block_size, "input");
    out = new digest_variable<FieldT>(pb, SHA256_block_size, "output");
    g = new sha256_two_to_one_hash_gadget<FieldT>(pb, SHA256_block_size, *inp, *out, "f");
    g->generate_r1cs_constraints();
    cout << "4444>>>>>>>> " << endl;
    printf("Number of constraints for sha256_two_to_one_hash_gadget: %zu\n", pb.num_constraints());

    // Trusted setup
    const r1cs_constraint_system<FieldT> constraint_system = pb.get_constraint_system();
    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(
            constraint_system);
    return keypair;
}

void two_inputs_hash_gadget() {
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

    // Empty string (all 0s)
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

    cout << "two_inputs_hash_gadget => Satisfied status: " << pb.is_satisfied() << endl;

    // Create proof
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof1 = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, pb.primary_input(), pb.auxiliary_input());

    // Verify proof
    bool verified1 = verify_proof(keypair.vk, pb.primary_input(), proof1);

    cout << "two_inputs_hash_gadget => Verfied: " << verified1 << endl;
}

void one_input_hash_gadget() {
    protoboard<FieldT> pb;
    block_variable<FieldT>* input;
    digest_variable<FieldT>* output;
    sha256_two_to_one_hash_gadget<FieldT>* f;

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = setup_gadget(pb, input, output, f);

    int i = 0;
//    std::chrono::duration<std::chrono::microseconds> tp = 0;
//    std::chrono::duration<std::chrono::microseconds> tv = 0;

    while (i < 10) {
        // Hash of string "hello world"
        const libff::bit_vector hash_bv = libff::int_list_to_bits({0xc082e440, 0x671cd799, 0x8baf04c0, 0x22c07e03, 0x4b125ee7, 0xd28e0a59, 0x49e4b924, 0x5f5cf897}, 32);
        output->generate_r1cs_witness(hash_bv);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // Add witness values
        // For string "hello world"
        const libff::bit_vector input_bv = libff::int_list_to_bits({0x6c6c6568, 0x6f77206f, 0x00646c72, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, 32);
        input->generate_r1cs_witness(input_bv);

        f->generate_r1cs_witness();

        cout << "one_input_hash_gadget => Satisfied status: " << pb.is_satisfied() << endl;

        // Create proof
        const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof1 = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
                keypair.pk, pb.primary_input(), pb.auxiliary_input());

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "Proving time: " << (end - begin).count() << endl;
//        tp += (end - begin).count();

        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        // Verify proof
        bool verified1 = verify_proof(keypair.vk, pb.primary_input(), proof1);
        std::chrono::steady_clock::time_point end1= std::chrono::steady_clock::now();
        cout << "Verification time: " << (end1 - begin1).count() << endl;
//        tv += (end1 - begin1);

        cout << "one_input_hash_gadget => Verfied: " << verified1 << endl;

        i++;
    }


}


int main() {
    // Initialize the curve parameters

    default_r1cs_ppzksnark_pp::init_public_params();

//    two_inputs_hash_gadget();

    one_input_hash_gadget();

    return 0;
}