
.PHONY: all clean cleanall

all:
	@$(MAKE) -C esma_murmur_hash
	@$(MAKE) -C esma_ring_buffer
	@$(MAKE) -C esma_mempool
	@$(MAKE) -C esma_array
	@$(MAKE) -C esma_cpu

clean:
	@$(MAKE) clean -C esma_murmur_hash
	@$(MAKE) clean -C esma_ring_buffer
	@$(MAKE) clean -C esma_mempool
	@$(MAKE) clean -C esma_array
	@$(MAKE) clean -C esma_cpu

cleanall:
	@$(MAKE) cleanall -C esma_murmur_hash
	@$(MAKE) cleanall -C esma_ring_buffer
	@$(MAKE) cleanall -C esma_mempool
	@$(MAKE) cleanall -C esma_array
	@$(MAKE) cleanall -C esma_cpu
