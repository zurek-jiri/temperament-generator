// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "Temperament.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <sstream>

namespace temperament
{
namespace
{
constexpr int64_t scale = 5354228880LL; // lcm(1, ..., 24): exact rational closure states.
struct Candidate
{
    int64_t p = 0, s = 0;
    double cents = 0;
    int complexity = 0;
    struct Atom { int n=0,d=1,unit=0; };
    std::array<Atom,2> atoms {};
    int count=0;
};
Candidate candidate(int n, int d, int unit, int m = 0, int e = 1, int second = 0)
{
    Candidate c;
    const auto add = [&](int num, int den, int u) {
        if (num == 0) return;
        const int gcd = std::gcd(std::abs(num), den);
        const int reducedN=num/gcd,reducedD=den/gcd;
        c.complexity += 30 + 4*std::abs(reducedN) + 2*static_cast<int>(std::ceil(std::log2(reducedD))) + (u==2?12:0);
        c.atoms[static_cast<size_t>(c.count++)]={reducedN,reducedD,u};
        const auto amount = static_cast<int64_t>(num) * (scale/den);
        if (u == 0 || u == 2) c.p += amount;
        if (u == 1) c.s += amount;
        if (u == 2) c.s -= amount;
    };
    add(n,d,unit); add(m,e,second);
    c.cents = static_cast<double>(c.p)/scale*pythagoreanComma() + static_cast<double>(c.s)/scale*syntonicComma();
    return c;
}
std::string renderCandidate(const Candidate& c)
{
    std::string result;
    for(int i=0;i<c.count;++i)
    {
        const auto a=c.atoms[static_cast<size_t>(i)];
        if(a.n==0) continue;
        result+=a.n<0?"-":"+";
        result+=a.unit==0?"P":a.unit==1?"S":"schisma";
        if(std::abs(a.n)!=1) result+="*"+std::to_string(std::abs(a.n));
        if(a.d!=1) result+="/"+std::to_string(a.d);
    }
    if(result.empty()) return "0";
    if(result.front()=='+') result.erase(0,1);
    return result;
}
template<class Visitor> void candidates(double target, Visitor visit)
{
    const double units[] { pythagoreanComma(), syntonicComma(), schisma() };
    visit(Candidate {});
    for (int u = 0; u < 3; ++u)
        for (int d = 1; d <= 24; ++d)
        {
            const int n=static_cast<int>(std::round(target/units[u]*d));
            if(std::abs(n)<=12 || std::abs(target)>12*pythagoreanComma()) visit(candidate(n,d,u));
        }
    // Two independent comma units suffice because H = P - S. Search bounded
    // rational coefficients; never promise recovery of an unknowable original.
    for (int u = 0; u < 3; ++u)
        for (int v = u+1; v < 3; ++v)
            for (int d = 1; d <= 24; ++d)
            {
                const double centre = target/units[u];
                for (int anchor = 0; anchor < 2; ++anchor)
                {
                    const double a = anchor == 0 ? centre : 0;
                    for (int n = static_cast<int>(std::floor((a-2)*d)); n <= static_cast<int>(std::ceil((a+2)*d)); ++n)
                    {
                        if (std::abs(n)>12 || std::gcd(std::abs(n), d) != 1) continue;
                        const double remaining = target - static_cast<double>(n)/d*units[u];
                        for (int e = 1; e <= 24; ++e)
                        {
                            const int m=static_cast<int>(std::round(remaining/units[v]*e));
                            if(std::abs(m)<=12) visit(candidate(n,d,u,m,e,v));
                        }
                    }
                }
            }
}
std::string term(double coefficient, const char* unit)
{
    if (std::abs(coefficient) < 1e-13) return {};
    const bool negative = coefficient < 0;
    coefficient = std::abs(coefficient);
    for (int d = 1; d <= 5760; ++d)
    {
        const auto n = static_cast<int64_t>(std::llround(coefficient*d));
        if (std::abs(coefficient-static_cast<double>(n)/d) > 2e-13) continue;
        return std::string(negative ? "-" : "+") + unit + (n == 1 ? "" : "*"+std::to_string(n))
            + (d == 1 ? "" : "/"+std::to_string(d));
    }
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << (negative ? "-" : "+") << unit << '*' << std::setprecision(17) << coefficient;
    return out.str();
}
std::string terms(double a, const char* u, double b, const char* v)
{
    auto text = term(a,u)+term(b,v);
    if (text.empty()) return "0";
    if (text.front() == '+') text.erase(0,1);
    return text;
}
using Key = std::pair<int64_t,int64_t>;
bool scaled(CommaComponents c, Key& key)
{
    if (!std::isfinite(c.p) || !std::isfinite(c.s) || std::abs(c.p)>100000 || std::abs(c.s)>100000) return false;
    key = { std::llround(c.p*scale), std::llround(c.s*scale) };
    return std::abs(c.p-static_cast<double>(key.first)/scale)<1e-12
        && std::abs(c.s-static_cast<double>(key.second)/scale)<1e-12;
}
// A rounded note has a half-step uncertainty of 0.0005 ct. A fifth is
// a difference of two such notes, so 0.001 ct is sufficient, not 1e-10 ct.
constexpr double csvFifthTolerance=0.00100001;
Candidate readableCandidate(double target,double tolerance=csvFifthTolerance)
{
    Candidate best;
    double bestError=std::numeric_limits<double>::infinity();
    candidates(target,[&](Candidate c) {
        const double error=std::abs(c.cents-target);
        const bool inside=error<=tolerance, bestInside=bestError<=tolerance;
        const bool better=inside!=bestInside?inside:
            inside?(c.complexity<best.complexity || (c.complexity==best.complexity&&error<bestError-1e-10)):
                   (error<bestError-1e-10 || (std::abs(error-bestError)<=1e-10&&c.complexity<best.complexity));
        if(better) {best=c;bestError=error;}
    });
    return best;
}
std::string closingRecipe(const std::array<std::string,12>& expressions,Mode mode,int pivot)
{
    CommaComponents remainder {-1,0};
    std::map<std::string,int> counts;
    for(int i=0;i<12;++i) if(i!=pivot)
    {
        CommaComponents c;
        if(!expressionComponents(expressions[i],mode,c)) return {};
        remainder.p-=c.p;remainder.s-=c.s;
        if(expressions[i]!="0") ++counts[expressions[i]];
    }
    const auto compact=commaExpression(remainder);
    // Avoid replacing small summands by giant combined numerators or decimals.
    bool small=compact.find('.')==std::string::npos;
    int number=0;
    for(char ch:compact)
    {
        if(ch>='0'&&ch<='9') {number=number*10+(ch-'0');if(number>24) small=false;}
        else number=0;
        if(!small) break;
    }
    if(small) return compact;
    std::string sum;
    for(const auto& entry:counts)
    {
        if(!sum.empty()) sum+='+';
        if(entry.second>1) sum+=std::to_string(entry.second)+"*";
        sum+="("+entry.first+")";
    }
    return sum.empty()?"-P":"-P-("+sum+")";
}
bool preserveSimpleFields(const ClosingRequest& request,const std::array<double,12>& targets,ClosingProposal& proposal,bool allowLong=false)
{
    auto expressions=request.expressions;
    std::array<Candidate,12> preferred {};
    double sum=pythagoreanComma();
    for(int i=0;i<12;++i)
    {
        if(request.selected[i])
        {
            preferred[i]=readableCandidate(targets[i],std::min(request.maximumChange,csvFifthTolerance));
            if(std::abs(preferred[i].cents-targets[i])>request.maximumChange+1e-9) return false;
            expressions[i]=renderCandidate(preferred[i]);
            sum+=preferred[i].cents;
        }
        else
        {
            double fraction=0;std::string error;
            if(!evaluateExpression(expressions[i],request.mode,fraction,error)) return false;
            sum+=fraction*commaSize(request.mode);
        }
    }
    if(std::abs(sum)<1e-6)
    {
        proposal.expressions=expressions;
        proposal.message="Simple expressions already close within calculation precision.";
        return true;
    }
    int bestPivot=-1;
    double bestScore=std::numeric_limits<double>::infinity();
    std::string bestRecipe;
    for(int i=0;i<12;++i) if(request.selected[i])
    {
        const double change=preferred[i].cents-sum-targets[i];
        if(std::abs(change)>request.maximumChange+1e-9) continue;
        auto recipe=closingRecipe(expressions,request.mode,i);
        if(recipe.empty() || recipe.size()>(allowLong?512u:80u)) continue;
        // Protect simple native fractions first. Concentrate the remainder in
        // the already more complicated field instead of corrupting every fifth.
        const double score=static_cast<double>(recipe.size())-2.0*preferred[i].complexity+std::abs(change);
        if(score<bestScore) {bestScore=score;bestPivot=i;bestRecipe=std::move(recipe);}
    }
    if(bestPivot<0) return false;
    expressions[bestPivot]=bestRecipe;
    proposal.expressions=expressions;
    proposal.message="Simple expressions retained; fifth "+std::to_string(bestPivot+1)+" carries the remaining closure correction. All changes respect the selected limit.";
    return true;
}
bool sharedSchismaFit(const ClosingRequest& request,const std::array<double,12>& targets,ClosingProposal& proposal)
{
    if(!std::all_of(request.selected.begin(),request.selected.end(),[](bool value) {return value;})) return false;
    struct State {double cost=0;std::array<int,12> multiples {};};
    double bestCost=std::numeric_limits<double>::infinity();
    std::array<std::string,12> best;
    for(int denominator=1;denominator<=24;++denominator)
    {
        std::map<int,State> states {{0,{}}};
        for(int edge=0;edge<12;++edge)
        {
            std::map<int,State> next;
            for(int k=-12;k<=12;++k)
            {
                const auto c=candidate(-1,12,0,k,denominator,2);
                const double error=c.cents-targets[edge];
                if(std::abs(error)>request.maximumChange+1e-9) continue;
                for(const auto& old:states)
                {
                    const int sum=old.first+k;
                    const double cost=old.second.cost+c.complexity+error*error/(request.maximumChange*request.maximumChange);
                    auto found=next.find(sum);
                    if(found==next.end()||cost<found->second.cost)
                    {
                        auto state=old.second;state.cost=cost;state.multiples[edge]=k;next[sum]=state;
                    }
                }
            }
            states=std::move(next);
            if(states.empty()) break;
        }
        const auto closed=states.find(0);
        if(closed!=states.end()&&closed->second.cost<bestCost)
        {
            bestCost=closed->second.cost;
            for(int i=0;i<12;++i) best[i]=renderCandidate(candidate(-1,12,0,closed->second.multiples[i],denominator,2));
        }
    }
    if(!std::isfinite(bestCost)) return false;
    proposal.expressions=best;
    proposal.message="Closed fit using small schisma adjustments to -P/12. Every fifth stays within the selected change limit.";
    return true;
}
bool simpleFit(const ClosingRequest& request, const std::array<CommaComponents,12>& components,
               const std::array<double,12>& targets, ClosingProposal& proposal)
{
    if (!std::isfinite(request.maximumChange) || request.maximumChange <= 0 || request.maximumChange > 100)
    { proposal.message = "Set a per-fifth change limit greater than 0 and at most 100 cents."; return false; }
    Key required { -scale, 0 };
    std::vector<int> edges;
    for (int i=0;i<12;++i)
        if (request.selected[i]) edges.push_back(i);
        else
        {
            Key fixed;
            if (!scaled(components[i],fixed))
            { proposal.message = "A fixed expression cannot be represented on the simple-fraction grid. Select that fifth too, or use equal adjustment."; return false; }
            required.first -= fixed.first; required.second -= fixed.second;
        }
    if (edges.empty()) { proposal.message = "Select at least one fifth to adjust."; return false; }
    if(preserveSimpleFields(request,targets,proposal)) return true;
    if(sharedSchismaFit(request,targets,proposal)) return true;
    struct Scored { Candidate c; double cost; bool operator<(const Scored& b) const { return cost < b.cost; } };
    std::vector<std::vector<Scored>> options;
    for (int edge : edges)
    {
        std::map<Key,Scored> unique;
        const double tolerance = request.maximumChange;
        candidates(targets[edge], [&](Candidate c) {
            const double error = c.cents-targets[edge];
            if (std::abs(error)>tolerance+1e-10) return;
            const double cost = error*error + tolerance*tolerance*0.0005*c.complexity;
            const Key key {c.p,c.s};
            auto it = unique.find(key);
            if (it == unique.end() || cost < it->second.cost) unique[key] = {c,cost};
        });
        std::vector<Scored> list;
        for (const auto& entry : unique) list.push_back(entry.second);
        std::sort(list.begin(),list.end(),[](const auto& a,const auto& b) {
            if (a.cost != b.cost) return a.cost < b.cost;
            return Key(a.c.p,a.c.s) < Key(b.c.p,b.c.s);
        });
        if (list.size()>48) list.resize(48);
        // Keep eligible menu fractions even when many intricate expressions
        // have a smaller local error. A global closure can require one of them
        // (for example twelve -P/12 corrections when starting from pure fifths).
        const auto retain=[&](Candidate native) {
            const double error=native.cents-targets[edge];
            if(std::abs(error)>tolerance+1e-10) return;
            if(std::none_of(list.begin(),list.end(),[&](const auto& entry) {return entry.c.p==native.p&&entry.c.s==native.s;}))
                list.push_back({native,error*error+tolerance*tolerance*0.0005*native.complexity});
        };
        retain(Candidate {});
        for(int unit=0;unit<3;++unit)
            for(int denominator:{1,2,3,4,5,6,12,24})
                for(int sign:{-1,1})
                    retain(candidate(sign,denominator,unit));
        if (list.empty()) { proposal.message = "No simple fraction lies within the selected change limit."; return false; }
        options.push_back(std::move(list));
    }
    struct State { Key sum; double cost; std::array<unsigned char,12> path {}; };
    std::vector<State> beam { {{0,0},0,{}} };
    const auto physical=[](Key key) {return static_cast<double>(key.first)/scale*pythagoreanComma()+static_cast<double>(key.second)/scale*syntonicComma();};
    const double requiredCents=physical(required);
    for (size_t depth=0; depth<edges.size(); ++depth)
    {
        int64_t minP=0,maxP=0,minS=0,maxS=0;
        double minCents=0,maxCents=0,remainingTargets=0;
        for (size_t j=depth+1;j<options.size();++j)
        {
            int64_t p0=INT64_MAX,p1=INT64_MIN,s0=INT64_MAX,s1=INT64_MIN;
            double c0=std::numeric_limits<double>::infinity(),c1=-std::numeric_limits<double>::infinity();
            for (const auto& o : options[j])
            {
                p0=std::min(p0,o.c.p);p1=std::max(p1,o.c.p);s0=std::min(s0,o.c.s);s1=std::max(s1,o.c.s);
                c0=std::min(c0,o.c.cents);c1=std::max(c1,o.c.cents);
            }
            minP+=p0;maxP+=p1;minS+=s0;maxS+=s1;
            minCents+=c0;maxCents+=c1;remainingTargets+=targets[edges[j]];
        }
        std::map<Key,State> next;
        for (const auto& state : beam)
            for (size_t n=0;n<options[depth].size();++n)
            {
                const auto& o = options[depth][n];
                const Key sum {state.sum.first+o.c.p,state.sum.second+o.c.s};
                if (required.first-sum.first<minP || required.first-sum.first>maxP
                    || required.second-sum.second<minS || required.second-sum.second>maxS) continue;
                const double remainingCents=requiredCents-physical(sum);
                if(remainingCents<minCents-1e-8||remainingCents>maxCents+1e-8) continue;
                const double cost = state.cost+o.cost;
                auto it = next.find(sum);
                if (it == next.end() || cost < it->second.cost)
                {
                    auto s = state;s.sum=sum;s.cost=cost;s.path[depth]=static_cast<unsigned char>(n);
                    next[sum]=s;
                }
            }
        beam.clear();
        for (const auto& entry : next) beam.push_back(entry.second);
        const auto lowerBound=[&](const State& state) {
            const double remaining=requiredCents-physical(state.sum)-remainingTargets;
            const auto count=edges.size()-depth-1;
            return state.cost+(count==0?0:remaining*remaining/static_cast<double>(count));
        };
        std::sort(beam.begin(),beam.end(),[&](const auto& a,const auto& b) {
            const double aBound=lowerBound(a),bBound=lowerBound(b);
            if (aBound != bBound) return aBound < bBound;
            return a.sum < b.sum;
        });
        if (beam.size()>4000) beam.resize(4000);
        if (beam.empty()) break;
    }
    for (const auto& state : beam)
        if (state.sum == required)
        {
            for (size_t j=0;j<edges.size();++j)
            {
                const auto& c=options[j][state.path[j]].c;
                proposal.expressions[edges[j]]=renderCandidate(c);
            }
            proposal.message = "Inferred simple fit: a bounded search balances small changes and readable fractions. This does not identify the original temperament uniquely.";
            return true;
        }
    if(preserveSimpleFields(request,targets,proposal,true)) return true;
    proposal.message = "No closed fit satisfies the selected change limit. Select more fifths or allow a larger change.";
    return false;
}
}

std::string commaExpression(CommaComponents c)
{
    auto best=terms(c.p,"P",c.s,"S");
    for (const auto& alternate : {terms(c.p+c.s,"P",-c.s,"schisma"),terms(c.p+c.s,"S",c.p,"schisma")})
        if (alternate.size()<best.size()) best=alternate;
    return best;
}

ExpressionChoice preciseExpression(double cents, Mode mode)
{
    if (!std::isfinite(cents) || std::abs(cents)>50000) return { "0",0 };
    const auto best=readableCandidate(cents);
    return { renderCandidate(best),best.cents/commaSize(mode) };
}

ClosingProposal proposeClosing(const ClosingRequest& request)
{
    ClosingProposal result;
    result.expressions=request.expressions;
    std::array<FifthCorrection,12> fields {};
    std::array<CommaComponents,12> components {};
    std::array<bool,12> linear {};
    CommaComponents fixedSum;
    int autos=0;
    for (int i=0;i<12;++i)
    {
        if (isAutomaticExpression(request.expressions[i])) { fields[i].automaticPrimary=true; ++autos; continue; }
        std::string error;
        if (!evaluateExpression(request.expressions[i],request.mode,fields[i].primary,error) || std::abs(fields[i].primary)>1000)
        { result.message="Fifth "+std::to_string(i+1)+": "+(error.empty()?"correction exceeds 1000 commas.":error); return result; }
        linear[i]=expressionComponents(request.expressions[i],request.mode,components[i])
            && std::isfinite(components[i].p) && std::isfinite(components[i].s)
            && std::abs(components[i].p)<=100000 && std::abs(components[i].s)<=100000;
        if(!linear[i]) components[i]={};
        fixedSum.p+=components[i].p;fixedSum.s+=components[i].s;
    }
    const auto resolved=resolveAutomatic(request.mode,fields);
    if (!resolved.valid) { result.message=resolved.error;return result; }
    bool allLinear=true;
    for (int i=0;i<12;++i) if (!fields[i].automaticPrimary && !linear[i]) allLinear=false;
    std::array<double,12> targets {};
    result.closureBefore=pythagoreanComma();
    for (int i=0;i<12;++i)
    {
        result.corrections[i]=resolved.fractions[i]*commaSize(request.mode);
        result.closureBefore+=result.corrections[i];
        targets[i]=request.useTargets?request.targets[i]:result.corrections[i];
        if (!std::isfinite(targets[i]) || std::abs(targets[i])>50000)
        { result.message="Invalid target fifth correction.";return result; }
        if (fields[i].automaticPrimary && allLinear) components[i]={(-1-fixedSum.p)/autos,-fixedSum.s/autos};
    }
    if (request.method==ClosingMethod::automaticFields)
    {
        if (autos==0) {result.message="Choose Automatic calculation in at least one formula field first.";return result;}
        result.message="Each automatic field receives the same cent adjustment. Fixed formulas are preserved.";
    }
    else if (request.method==ClosingMethod::simpleFit)
    {
        if (!allLinear) {result.message="Simple fitting requires linear P, S or schisma expressions. Use equal adjustment for nonlinear formulas.";return result;}
        if (!simpleFit(request,components,targets,result)) return result;
    }
    else
    {
        int count=0;
        for (bool selected:request.selected) if(selected || request.method==ClosingMethod::allEqually) ++count;
        if (count==0) {result.message="Select at least one fifth to adjust.";return result;}
        CommaComponents total;
        for (auto c:components) {total.p+=c.p;total.s+=c.s;}
        for (int i=0;i<12;++i)
            if (request.selected[i] || request.method==ClosingMethod::allEqually)
            {
                if (allLinear) result.expressions[i]=commaExpression({components[i].p+(-1-total.p)/count,components[i].s-total.s/count});
                else result.expressions[i]=fractionExpression((result.corrections[i]-result.closureBefore/count)/commaSize(request.mode),request.mode);
            }
        result.message="Closure correction shared equally in cents across "+std::to_string(count)+" selected fifth(s).";
    }
    // Materialise any remaining Auto fields for explicit adjustment methods;
    // otherwise they would move after a neighbouring field has been changed.
    if (request.method!=ClosingMethod::automaticFields)
        for (int i=0;i<12;++i) if (isAutomaticExpression(result.expressions[i]))
            result.expressions[i]=allLinear?commaExpression(components[i]):fractionExpression(resolved.fractions[i],request.mode);
    std::array<FifthCorrection,12> proposed {};
    for (int i=0;i<12;++i)
    {
        std::string error;
        proposed[i].automaticPrimary=isAutomaticExpression(result.expressions[i]);
        if (!proposed[i].automaticPrimary && !evaluateExpression(result.expressions[i],request.mode,proposed[i].primary,error))
        {result.message="Could not express the proposed correction: "+error;return result;}
    }
    const auto final=resolveAutomatic(request.mode,proposed);
    const auto chart=calculate(request.mode,{std::vector<double>(final.fractions.begin(),final.fractions.end())});
    if (!final.valid || !chart.valid) {result.message="The proposed expressions did not close accurately. "+chart.error;return result;}
    result.closureAfter=chart.closureErrors.front();
    for (int i=0;i<12;++i)
    {
        result.corrections[i]=final.fractions[i]*commaSize(request.mode);
        result.changes[i]=result.corrections[i]-targets[i];
        result.maximumChange=std::max(result.maximumChange,std::abs(result.changes[i]));
        if (request.method==ClosingMethod::simpleFit && request.selected[i] && std::abs(result.changes[i])>request.maximumChange+1e-7)
        {result.message="Proposed fit exceeds the per-fifth change limit.";return result;}
    }
    result.valid=true;
    return result;
}
}
