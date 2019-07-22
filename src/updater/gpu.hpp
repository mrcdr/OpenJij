//    Copyright 2019 Jij Inc.
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at

//        http://www.apache.org/licenses/LICENSE-2.0

//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#ifndef OPENJIJ_UPDATER_GPU_HPP__
#define OPENJIJ_UPDATER_GPU_HPP__

#ifdef USE_CUDA

#include <system/gpu/chimera_gpu_transverse.hpp>
#include <utility/schedule_list.hpp>
#include <utility/random.hpp>
#include <system/gpu/chimera_cuda/kernel.hpp>

namespace openjij {
    namespace updater {

        /**
         * @brief GPU algorithm using cuda
         *
         * @tparam System type of system
         */
        template<typename System>
        struct GPU;

        /**
         * @brief GPU algorithm for chimera transverse model
         *
         */
        template<typename FloatType,
            std::size_t rows_per_block,
            std::size_t cols_per_block,
            std::size_t trotters_per_block>
        struct GPU<system::ChimeraTransverseGPU<FloatType, rows_per_block, cols_per_block, trotters_per_block>> {
            
            /**
             * @brief Chimera Transverse type
             */
            using QIsing = system::ChimeraTransverseGPU<FloatType, rows_per_block, cols_per_block, trotters_per_block>;
            /**
             * 
             * @brief operate GPU monte carlo in a chimera transverse ising system
             *
             * @param system object of a chimera transverse system
             * @param random_number_engine random number gengine
             * @param parameter parameter object including inverse temperature \f\beta:=(k_B T)^{-1}\f
             *
             * @return energy difference \f\Delta E\f
             */
          template<curandRngType_t rng_type>
            inline static FloatType update(QIsing& system,
                                 utility::cuda::CurandWrapper<FloatType, rng_type>& random_number_engine,
                                 const utility::TransverseFieldUpdaterParameter& parameter) {

                static auto dE = utility::cuda::make_dev_unique<FloatType[]>(1);

                FloatType ret_dE = 0;
                //initialize dE
                HANDLE_ERROR_CUDA(cudaMemcpy(dE.get(), &ret_dE, 1*sizeof(FloatType), cudaMemcpyHostToDevice));
                //generate uniform random sequence
                random_number_engine.generate_uniform(system.info.rows*system.info.cols*system.info.trotters*system.info.chimera_unitsize);

                //do metropolis
                system::chimera_cuda::metropolis_interface<FloatType, rows_per_block, cols_per_block, trotters_per_block>(
                        system.spin.get(), random_number_engine.get(),
                        dE.get(),
                        system.interaction.J_out_p.get(),
                        system.interaction.J_out_n.get(),
                        system.interaction.J_in_04.get(),
                        system.interaction.J_in_15.get(),
                        system.interaction.J_in_26.get(),
                        system.interaction.J_in_37.get(),
                        system.interaction.h.get(),
                        system.info, system.grid, system.block,
                        parameter.beta, system.gamma, parameter.s
                        );


                //retrieve dE
                HANDLE_ERROR_CUDA(cudaMemcpy(&ret_dE, dE.get(), 1*sizeof(FloatType), cudaMemcpyDeviceToHost));

                return ret_dE;
            }
        };

        

    } // namespace updater
} // namespace openjij

#endif

#endif
