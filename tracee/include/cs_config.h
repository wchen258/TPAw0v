#ifndef CS_CONFIG_H
#define CS_CONFIG_H

void cs_config_etr(uint64_t buf_addr, uint32_t buf_size);
void cs_config_etr_mp(uint64_t buf_addr, uint32_t buf_size);
void config_etm(void);
void config_etm_n(ETM_interface* etm_n, int stall, int id);
void config_etm_addr_event_test(ETM_interface*, uint64_t, uint64_t, uint64_t, uint64_t);
void config_etm_single_pmu_event_test(ETM_interface*, int event_bus_num);





#endif