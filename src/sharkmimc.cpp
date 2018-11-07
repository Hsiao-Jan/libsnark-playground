#include <stdlib.h>
#include <iostream>
#include <chrono>

#include "sharkmimc.hpp"

using namespace std;
using namespace std::chrono;

typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;


int main() {
    default_r1cs_ppzksnark_pp::init_public_params();

    protoboard<FieldT> pb;
    pb_variable_array<FieldT> input;

    cout << "Mod is " << FieldT::mod << endl;
    cout << "No of bits is " << FieldT::num_bits << endl;
    cout << "No of limbs is " << FieldT::num_limbs << endl;


    input.allocate(pb, 4, "input");
    auto i0 = FieldT("21871881226116355513319084168586976250335411806112527735069209751513595455673");
    auto i1 = FieldT("55049861378429053168722197095693172831329974911537953231866155060049976290");
    auto i2 = FieldT("21871881226116355513319084168586976250335411806112527735069209751513595455673");
    auto i3 = FieldT("55049861378429053168722197095693172831329974911537953231866155060049976290");

    SharkMimc_inverse_gadget<FieldT> g_inv(FieldT::mod, pb, input, "Sharkmimc inverse gadget");

    g_inv.prepare_round_constants();
    g_inv.prepare_matrix_1();
    g_inv.prepare_matrix_2();
    g_inv.prepare_round_keys();
    g_inv.generate_r1cs_constraints();

    cout << "Number of constraints: " << pb.num_constraints() << endl;

    // Trusted setup
    const r1cs_constraint_system<FieldT> constraint_system = pb.get_constraint_system();

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(
            constraint_system);

    duration<double> tc(0);
    duration<double> tp(0);
    steady_clock::time_point begin = steady_clock::now();

    /*input.resize(4);
    input.fill_with_field_elements(pb, field_elems);*/
    pb.val(input[0]) = i0;
    pb.val(input[1]) = i1;
    pb.val(input[2]) = i2;
    pb.val(input[3]) = i3;

    g_inv.generate_r1cs_witness();

    steady_clock::time_point mid = steady_clock::now();
    tc += duration_cast<duration<double>>(mid - begin);

    cout << "Satisfied status: " << pb.is_satisfied() << endl;
    /*cout << "Result: " << endl;
    cout << pb.val(g_inv.result()[0]) << endl;
    cout << pb.val(g_inv.result()[1]) << endl;
    cout << pb.val(g_inv.result()[2]) << endl;
    cout << pb.val(g_inv.result()[3]) << endl;*/

    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, pb.primary_input(), pb.auxiliary_input());

    steady_clock::time_point end = steady_clock::now();
    tp += duration_cast<duration<double>>(end - begin);

    cout << "Total constraint generation time (seconds): " << tc.count() << endl;
    cout << "Total proving time (seconds): " << tp.count() << endl;
    return 0;
}