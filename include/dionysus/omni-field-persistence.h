#ifndef DIONYSUS_OMNI_FIELD_REDUCTION_H
#define DIONYSUS_OMNI_FIELD_REDUCTION_H

#include <vector>
#include <unordered_map>

#include "reduction.h"      // for unpaired
#include "fields/q.h"
#include "fields/zp.h"
#include "chain.h"

namespace dionysus
{

template<typename Index_ = unsigned, class Comparison_ = std::less<Index_>>
class OmniFieldPersistence
{
    public:
        using   Index       = Index_;
        using   Q           = ::dionysus::Q<>;
        using   Field       = Q;
        using   Comparison  = Comparison_;

        using   BaseElement = Q::BaseElement;
        using   Zp          = ::dionysus::ZpField<BaseElement>;
        using   Zps         = std::unordered_map<BaseElement, Zp>;

        using   QElement    = Q::Element;
        using   QEntry      = ChainEntry<Q,Index>;
        using   QChain      = std::vector<QEntry>;

        using   ZpElement   = Zp::Element;
        using   ZpEntry     = ChainEntry<Zp, Index>;
        using   ZpChain     = std::vector<ZpEntry>;

        using   QChains     = std::vector<QChain>;
        using   ZpChains    = std::unordered_map<Index, std::unordered_map<BaseElement, ZpChain>>;

        using   QLows       = std::unordered_map<Index, Index>;
        using   ZpLows      = std::unordered_map<Index, std::unordered_map<BaseElement, Index>>;

        using   QPairs      = std::vector<Index>;
        using   ZpPairs     = std::unordered_map<BaseElement, std::unordered_map<Index, Index>>;

        using   Factors     = std::vector<BaseElement>;

        const Field&        field() const                       { return q_; }

        void                sort(QChain& c)                     { std::sort(c.begin(), c.end(),
                                                                  [this](const QEntry& e1, const QEntry& e2)
                                                                  { return this->cmp_(e1.index(), e2.index()); }); }

        template<class ChainRange>
        void                add(const ChainRange& chain)        { return add(QChain(std::begin(chain), std::end(chain))); }
        void                add(QChain&& chain);

        void                reserve(size_t s)                   { q_chains_.reserve(s); q_pairs_.reserve(s); }
        size_t              size() const                        { return q_pairs_.size(); }

        void                reduce(ZpChain& zp_chain, BaseElement p);
        ZpChain             convert(const QChain& c, const Zp& field) const;
        bool                special(Index i, BaseElement p) const   { auto it = zp_chains_.find(i); if (it == zp_chains_.end()) return false; if (it->second.find(p) == it->second.end()) return false; return true; }

        const Zp&           zp(BaseElement p) const             { auto it = zps_.find(p); if (it != zps_.end()) return it->second; return zps_.emplace(p, Zp(p)).first->second; }

        static Factors      factor(BaseElement x);

        const QChains&      q_chains() const                    { return q_chains_; }
        const ZpChains&     zp_chains() const                   { return zp_chains_; }

        // This is a bit of a hack; it takes advantage of the fact that zp(p)
        // generates field on-demand and memoizes them. So there is an entry in
        // zps_ only if something special happened over the prime.
        Factors             primes() const                      { Factors result; result.reserve(zps_.size()); for (auto& x : zps_) result.push_back(x.first); return result; }

        // TODO: no skip support for now
        bool                skip(Index) const                   { return false; }
        void                add_skip()                          {}
        void                set_skip(Index, bool flag = true)   {}

        Index               pair(Index i, BaseElement p) const;
        void                set_pair(Index i, Index j);
        void                set_pair(Index i, Index j, BaseElement p);
        static const Index  unpaired()                          { return Reduction<Index>::unpaired; }

    private:
        QChains     q_chains_;
        ZpChains    zp_chains_;

        QLows       q_lows_;
        ZpLows      zp_lows_;

        QPairs      q_pairs_;
        ZpPairs     zp_pairs_;

        Q           q_;
        mutable Zps zps_;

        Comparison  cmp_;
};

// Make OmniFieldPersistence act like a ReducedMatrix (e.g., for the purpose of constructing a persistence diagram)
template<typename Index_, class Comparison_>
struct PrimeAdapter
{
    using Persistence = OmniFieldPersistence<Index_, Comparison_>;
    using Prime       = typename Persistence::BaseElement;
    using Index       = typename Persistence::Index;

                        PrimeAdapter(const Persistence& persistence, Prime p):
                            persistence_(persistence), p_(p)    {}

    bool                skip(Index i) const                     { return persistence_.skip(i); }

    size_t              size() const                            { return persistence_.size(); }
    Index               pair(Index i) const                     { return persistence_.pair(i, p_); }
    static const Index  unpaired()                              { return Persistence::unpaired(); }

    const Persistence&  persistence_;
    Prime               p_;
};

template<typename Index, class Comparison>
PrimeAdapter<Index, Comparison>
prime_adapter(const OmniFieldPersistence<Index, Comparison>&    persistence,
              typename PrimeAdapter<Index, Comparison>::Prime   p)
{
    return PrimeAdapter<Index, Comparison>(persistence, p);
}

} // dionysus

#include "omni-field-persistence.hpp"

#endif
