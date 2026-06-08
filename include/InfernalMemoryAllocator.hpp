/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstddef>
#include <atomic>
#include <cstdint>
#include <new>
#include <algorithm>

#if defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE inline
#endif

template <typename KGN_Type>
	class InfernalMemoryAllocator {
		private:
			static constexpr size_t Memory_Alignment = 
				alignof(KGN_Type) > alignof(std::max_align_t) ? alignof(KGN_Type) : alignof(std::max_align_t);
				
			static constexpr size_t Total_Aligned_Bytes = 
				(sizeof(KGN_Type) + Memory_Alignment - 1) & ~(Memory_Alignment - 1);

			struct MemoryBlock { MemoryBlock* Next_Free_Block; }; // Next_Free_Block -> Current_Block.

			struct alignas(16) ABAProtectedPtr {
				MemoryBlock* Current_Block;
				uintptr_t ABA_Signature;
			};

			struct PrivateAddressReserve {
				MemoryBlock* Top_of_Chain = nullptr;
				uint32_t Chain_Length = 0;
			};

			static constexpr uint32_t Reserve_Limit = 1024;
			inline static thread_local PrivateAddressReserve L1_Address_Provider;
			void* BRUTE_MEMORY_BLOCK;
			alignas(128) std::atomic<ABAProtectedPtr> Atomic_Anchor; // I aligned it to 128 bytes to eliminate False Sharing.

		public:
            InfernalMemoryAllocator(size_t Total_Bytes) {
                BRUTE_MEMORY_BLOCK = ::operator new(Total_Aligned_Bytes * Total_Bytes, std::align_val_t{Memory_Alignment} );
                MemoryBlock* Aligned_Mem_Block_Start = static_cast<MemoryBlock*>(BRUTE_MEMORY_BLOCK);

                for (size_t K = 0; K < Total_Bytes - 1; ++K) {
                    MemoryBlock* Current_Block_Ptr =
                    	reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(Aligned_Mem_Block_Start) + (K * Total_Aligned_Bytes) );
                    MemoryBlock* Next_Free_Block =
                    	reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*> (Aligned_Mem_Block_Start) + ((K + 1) * Total_Aligned_Bytes) );
                    Current_Block_Ptr->Next_Free_Block = Next_Free_Block;
                } // Loop.

                MemoryBlock* Last_Block_Ptr =
                	reinterpret_cast<MemoryBlock*>(
                        reinterpret_cast<char*>(Aligned_Mem_Block_Start) + ((Total_Bytes - 1) * Total_Aligned_Bytes) );
                Last_Block_Ptr->Next_Free_Block = nullptr;
                Atomic_Anchor.store({Aligned_Mem_Block_Start, 0}, std::memory_order_release);
            } // InfernalMemoryAllocator.

            ~InfernalMemoryAllocator() { ::operator delete(BRUTE_MEMORY_BLOCK, std::align_val_t{Memory_Alignment} ); }

            InfernalMemoryAllocator(const InfernalMemoryAllocator&) = delete;
            InfernalMemoryAllocator& operator=(const InfernalMemoryAllocator&) = delete;

            [[nodiscard]] FORCE_INLINE KGN_Type* Allocate() noexcept {
                if (L1_Address_Provider.Chain_Length > 0) {
                    MemoryBlock* First_Chain_ADDRESS = L1_Address_Provider.Top_of_Chain;
                    L1_Address_Provider.Top_of_Chain = First_Chain_ADDRESS->Next_Free_Block;
                    L1_Address_Provider.Chain_Length--;
                    return reinterpret_cast<KGN_Type*>(First_Chain_ADDRESS);
                } // Se.
                return Provide_New_Batch_Address();
            } // Allocate();

            KGN_Type* Provide_New_Batch_Address() noexcept {
                ABAProtectedPtr Last_Address_Allocate = Atomic_Anchor.load(std::memory_order_acquire);
                ABAProtectedPtr Address_Connection;

                do {
                    if (!Last_Address_Allocate.Current_Block) return nullptr;
                    
                    MemoryBlock* Current_Free_Block = Last_Address_Allocate.Current_Block;
                    uint32_t Block_Count = 1;
                    
                    while (Block_Count < Reserve_Limit && Current_Free_Block->Next_Free_Block) {
                        Current_Free_Block = Current_Free_Block->Next_Free_Block;
                        Block_Count++;
                    } // Enquanto.

                    Address_Connection = {Current_Free_Block->Next_Free_Block, Last_Address_Allocate.ABA_Signature + 1};

                    if (Atomic_Anchor.compare_exchange_weak(
                    	Last_Address_Allocate, Address_Connection,
                        std::memory_order_acq_rel, std::memory_order_acquire) ) {

                        MemoryBlock* Saved_First_Address_Mem_Block = Last_Address_Allocate.Current_Block;
                        L1_Address_Provider.Top_of_Chain = Saved_First_Address_Mem_Block->Next_Free_Block;
                        L1_Address_Provider.Chain_Length = Block_Count - 1;
                        Current_Free_Block->Next_Free_Block = nullptr;
                        return reinterpret_cast<KGN_Type*>(Saved_First_Address_Mem_Block);
                    } // Se.
                } while (1);
            } // Provide_New_Batch_Address();

            FORCE_INLINE void Deallocate(KGN_Type* Ptr) noexcept {
                if (!Ptr) return;
                MemoryBlock* Node = reinterpret_cast<MemoryBlock*>(Ptr);
                Node->Next_Free_Block = L1_Address_Provider.Top_of_Chain;
                L1_Address_Provider.Top_of_Chain = Node;
                L1_Address_Provider.Chain_Length++;
            } //Deallocate();

};	// Class InfernalMemoryAllocator.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
