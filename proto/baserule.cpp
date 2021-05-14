#include "baserule.hpp"

namespace Comperator{    
  bool cmp_byte(const std::pair<uint32_t, Counter>& a, const std::pair<uint32_t, Counter>& b)
    { 
    return a.second.count_bytes < b.second.count_bytes; 
    }
  bool cmp_packet(const std::pair<uint32_t, Counter>& a, const std::pair<uint32_t, Counter>& b)
    { 
    return a.second.count_packets < b.second.count_packets; 
    }
}

// class _Counter
Counter::Counter():count_packets(0),count_bytes(0),pps(0),bps(0),pps_not_trig(0)
    ,bps_not_trig(0),last_update_(std::chrono::high_resolution_clock::now()){};    
Counter& Counter::operator=(const Counter& other){
    count_packets=other.count_packets;
    count_bytes=other.count_bytes;
    pps=other.pps;
    bps=other.bps;
    pps_not_trig=other.pps_not_trig;
    bps_not_trig=other.bps_not_trig;
    last_update_=other.last_update_;
    return *this;
}
Counter::Counter(const Counter & other){
    
    count_packets=other.count_packets;
    count_bytes=other.count_bytes; 
    pps=other.pps;
    bps=other.bps;
    pps_not_trig=other.pps_not_trig;
    bps_not_trig=other.bps_not_trig;
    last_update_=other.last_update_;
            
}

// class CountersList
template<class Key, class Val>
CountersList<Key, Val>::CountersList() {};
template<class Key, class Val>
CountersList<Key, Val>& CountersList<Key, Val>::operator+=(CountersList& other)
{
    if (this != &other)
    {
        for(auto& om: other.map_)
        {
            map_[om.first].count_bytes += om.second.count_bytes;
            map_[om.first].count_packets+=om.second.count_packets;                        
        }            
        other.clear();
    }
    return *this;
}
template<class Key, class Val>
void CountersList<Key, Val>::print() const
{
    for(auto& m: map_) {
        std::cout << "first: " << m.first << " second: " << m.second.count_packets << std::endl;
    }
}
template<class Key, class Val>
unsigned int CountersList<Key, Val>::size() const
{
    return map_.size();
}
template<class Key, class Val>
void CountersList<Key, Val>::clear()
{
    map_.clear();
}
template<class Key, class Val>
void CountersList<Key, Val>::increase(const Key& k)
{
    map_[k].count_packets++;
}
template<class Key, class Val>
void CountersList<Key, Val>::_increase(const Key& k, const unsigned int len)
{
    
    auto it= map_.find(k);
    if(it != map_.end()){
    
     map_[k].count_bytes+=len;
     map_[k].count_packets++;
    }
    else
    {
    Counter tmp;
    tmp.count_bytes=len;
    tmp.count_packets++;
    map_[k]=tmp;        
    }    
}
template<class Key, class Val>
std::string CountersList<Key, Val>::get_max() const
{
    std::pair<Key, Counter> max_val;
    for(auto& m: map_)
    {   
        if(m.second.count_packets > max_val.second.count_packets)
        {
            max_val = m;
        }
    }
    return boost::asio::ip::address_v4(max_val.first).to_string();
}
template<class Key, class Val>
void CountersList<Key,Val>::calc_delta(const CountersList& old){
    
    double delta_time;
    for(auto &om: old.map_){
        auto it=map_.find(om.first);
        if(it != map_.end()){
         delta_time = std::chrono::duration<double, std::milli>(
                map_[om.first].last_update_ - om.second.last_update_).count();
         map_[om.first].bps= round( ((map_[om.first].count_bytes - om.second.count_bytes)/delta_time) *1000 );
         map_[om.first].pps= round( ((map_[om.first].count_packets - om.second.count_packets)/delta_time) *1000 );
        }
        
    }    
}
template<class Key, class Val>
std::vector<std::string> CountersList<Key,Val>::get_ip_list(uint32_t pps_trig, unsigned int pps_period, uint32_t bps_trig, unsigned int bps_period){
  
    std::time_t cur_time = std::time(0);
    std::vector<std::pair<uint32_t, Counter> > A;
    std::vector<std::string> B;
    // Packet trigger
    if(pps_trig > 0)
    {
        
        for(auto& it: map_)
            A.push_back(it);
        
        std::sort(A.begin(),A.end(),Comperator::cmp_packet);        
        for(int i=A.size()-1; i>=0;i--)
        {              
            if(A[i].second.pps > pps_trig)
            {                                                        
                
                if((cur_time - A[i].second.pps_not_trig) > (std::time_t) pps_period)                    
                        A[i].second.pps_not_trig = cur_time; // So that the trigger fires once in a period                                                            
                B.push_back(boost::asio::ip::address_v4(A[i].first).to_string());                   
            }
            else
                return B;
        }
    }       
    // Trigger bytes
    if(bps_trig > 0)
    {        
        for(auto& it: map_)
            A.push_back(it);
        std::sort(A.begin(),A.end(),Comperator::cmp_byte);        
        for(int i=A.size()-1; i>=0;i--)
        {        
            if(A[i].second.bps > bps_trig)
            {                
                if((cur_time - A[i].second.bps_not_trig) > (std::time_t) bps_period)                 
                    A[i].second.bps_not_trig = cur_time;               
                B.push_back(boost::asio::ip::address_v4(A[i].first).to_string());                   
            }
            else
                return B;
               
        }           
    }
    return B;
}

// class NumRange
template<class T>
NumRange<T>::NumRange()
    : start_(0), end_(0), enable_(false) {}
template<class T>
NumRange<T>::NumRange(const std::pair<T, T>& p)
    : start_(p.first), end_(p.second), enable_(true) {}
template<class T>
bool NumRange<T>::in_this(T num) const
{
    if(!enable_)
        return true;
    if(num != 0 && num >= start_ && num <= end_)
    {
        return true;
    }
    else
    {
        return false;
    }
}
template<class T>
bool NumRange<T>::stat() const
{
    return enable_;
}
template<class T>
std::string NumRange<T>::to_cidr() const
{
    return boost::asio::ip::address_v4(start_).to_string() + "-" + boost::asio::ip::address_v4(end_).to_string();
}
template<class T>
std::string NumRange<T>::to_range() const
{
    return std::to_string(start_) + "-" + std::to_string(end_);
}
template<class T>
bool NumRange<T>::operator==(NumRange const & other) const
{
    return (start_==other.start_ && end_==other.end_);
}
template<class T>
NumRange<T>& NumRange<T>::operator=(const std::pair<T, T>& p)
{
    if(p.first != 0 || p.second != 0)
    {
        start_ = p.first;
        end_ = p.second;
        enable_ = true;
    }
    return *this;
}

// class NumComparable
template<class T>
bool NumComparable<T>::stat() const
{
    return enable_;
}
template<class T>
NumComparable<T>::NumComparable()
    : num_(0), enable_(false), type_(0) {}
template<class T>
NumComparable<T>::NumComparable(const std::pair<T, unsigned short int>& p)
    : num_(p.first), enable_(true), type_(p.second) {}
template<class T>
bool NumComparable<T>::in_this(T num) const
{
    if(!enable_)
        return true;
    if(type_ == 0 && num == num_)
        return true;
    if(type_ == 1 && num > num_)
        return true;
    if(type_ == 2 && num < num_)
        return true;
    return false;
}
template<class T>
std::string NumComparable<T>::to_str() const
{
    return std::to_string(type_) + ":" + std::to_string(num_);
}
template<class T>
bool NumComparable<T>::operator==(NumComparable const & other) const
{
    return (num_==other.num_ && type_==other.type_);
}
template<class T>
NumComparable<T>& NumComparable<T>::operator=(const std::pair<T, unsigned short int>& p)
{
    num_ = p.first;
    type_ = p.second;
    enable_ = true;
    return *this;
}

// struct BaseRule
BaseRule::BaseRule()
    : rule_type("none"), comment(""), count_packets(0), count_bytes(0),
    next_rule(false), pps(0), bps(0), pps_trigger(0), bps_trigger(0),
    pps_last_not_triggered(0), bps_last_not_triggered(0),
    pps_trigger_period(10), bps_trigger_period(10) {}
BaseRule::BaseRule(const std::vector<std::string>& tkn_rule)
    : rule_type("none"), comment(""), count_packets(0), count_bytes(0),
    next_rule(false), pps(0), bps(0), pps_trigger(0), bps_trigger(0),
    pps_last_not_triggered(0), bps_last_not_triggered(0),
    pps_trigger_period(10), bps_trigger_period(10), tokenize_rule(tkn_rule) {}
void BaseRule::BaseRule_parse(const boost::program_options::variables_map& vm)
{
    if(vm.count("action") && vm.count("filter")) {
	throw ParserException("only action or filter is allowed");	
    }
    if (vm.count("pps-th")) {
        pps_trigger = parser::from_short_size(vm["pps-th"].as<std::string>(), false);
    }
    if (vm.count("bps-th")) {
        bps_trigger = parser::from_short_size(vm["bps-th"].as<std::string>());
    }
    if (vm.count("pps-th-period")) {
        pps_trigger_period = vm["pps-th-period"].as<unsigned int>();
    }
    if (vm.count("bps-th-period")) {
        bps_trigger_period = vm["bps-th-period"].as<unsigned int>();
    }
    if (vm.count("action")) {
        act = parser::action_from_string(vm["action"].as<std::string>());
    }
    if(vm.count("filter")) {
	act = parser::action_from_string("filter");
    }
    if (vm.count("comment")) {
        comment = vm["comment"].as<std::string>();
    }
    if (vm.count("next")) {
        next_rule = vm.count("next");
    }
    // Checking required parameters
    if(pps_trigger == 0 && bps_trigger == 0)
        throw ParserException("pps or bps trigger will be set");
    if(pps_trigger > 0 && pps_trigger_period < 1)
        throw ParserException("incorrect pps trigger period");
    if(bps_trigger > 0 && bps_trigger_period < 1)
        throw ParserException("incorrect bps trigger period");
}
bool BaseRule::is_triggered()
{
    std::time_t cur_time = std::time(0);
    // Packet trigger
    if(pps_trigger > 0)
    {
        if(pps > pps_trigger)
        {
            // if (current time - last good check) > trigger piriod
            if((cur_time - pps_last_not_triggered) > (std::time_t) pps_trigger_period) 
            {
                pps_last_not_triggered = cur_time; // So that the trigger fires once in a period
                if(dst_top.size() > 0) // If the destination is known
                {
                    return true;
                }
            }
        }
        else
        {
            pps_last_not_triggered = cur_time;
        }
    }
    // Trigger bytes
    if(bps_trigger > 0)
    {
        if(bps > bps_trigger)
        {
            // if (current time - last good check) > trigger piriod
            if((cur_time - bps_last_not_triggered) > (std::time_t) bps_trigger_period) 
            {
                bps_last_not_triggered = cur_time; // чтобы триггер срабатывал один раз в период
                if(dst_top.size() > 0) // если адрес назначения известен
                {
                    return true;
                }
            }
        }
        else
        {
            bps_last_not_triggered = cur_time;
        }
    }
    return false;
}
void BaseRule::calc_delta(const BaseRule & old){
    
    dst_top.calc_delta(old.dst_top);
    
}
void BaseRule::get_ip_list(std::vector<std::string> & vect, const std::string& description){
     
    std::vector<std::string> tmp = dst_top.get_ip_list(pps_trigger,pps_trigger_period,bps_trigger,bps_trigger_period);   
    std::string info;
    for(unsigned int i=0; i< tmp.size();i++){
	    if(act.is_filter()){
		    info = "ANOMALY IP " + tmp[i] + "/32 " + "RULE " +
			    rule_type + " " + text_rule + "\n";
	    }
	    else{

        	info = rule_type + "|" + ((description.length()>0)?(description):"|")
       	 	+ tmp[i]
        	+ (comment == "" ? "" : "|" + comment);
	    }

        vect.push_back(info);        
    }
    
}
std::string BaseRule::get_job_info(std::string txt) const
{
    
    // std::string info = std::to_string(count_packets) + "|"
    //                 + std::to_string(count_bytes) + "|"
    //                 + std::to_string(pps) + "|"
    //                 + std::to_string(bps) + "|max: "
    //                 + dst_top.get_max()
    //                 + (comment == "" ? "" : "|" + comment);
    // return info;
    

    std::string info = rule_type + "|" + ((txt.length()>0)?(txt):"|")
        + dst_top.get_max()
        + (comment == "" ? "" : "|" + comment)/* + "|"
        + (ip_src.stat() ? ip_src.to_cidr() : "") + "|"
        + (ip_dst.stat() ? ip_dst.to_cidr() : "") + "|"
        + (src_port.stat() ? src_port.to_range() : "") + "|"
        + (dst_port.stat() ? dst_port.to_range() : "") + "|"*/;
    return info;
}
std::string BaseRule::get_trigger_influx() const
{
    std::string info = "events,dst=" + dst_top.get_max()
        + " bps=" + std::to_string(bps * 8)
        + ",pps=" + std::to_string(pps)
        + ",type=\"" + rule_type
        + "\",comment=\"" + (comment)
        + "\"";
    return info;
}

template class CountersList<uint16_t, unsigned int>;
template class CountersList<uint32_t, unsigned int>;
template class NumRange<uint16_t>;
template class NumRange<uint32_t>;
template class NumComparable<uint8_t>;
template class NumComparable<uint16_t>;
template class NumComparable<uint32_t>;

