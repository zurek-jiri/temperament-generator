// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "Temperament.h"
#include "Harmony.h"
#include "KnownTemperaments.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

using namespace temperament;
static int checks=0;
static void check(bool condition,const char* message)
{
    ++checks;if(!condition) {std::cerr<<"FAILED: "<<message<<'\n';std::exit(1);}
}
static double physical(const std::string& text,Mode mode=Mode::pythagoreanFifths)
{
    double value=0;std::string error;
    check(evaluateExpression(text,mode,value,error),"Generated expression parses");return value*commaSize(mode);
}
static void verify(const ClosingRequest& request,const ClosingProposal& proposal)
{
    if(!proposal.valid) std::cerr<<proposal.message<<'\n';
    check(proposal.valid,"Closing proposal is valid");
    std::array<FifthCorrection,12> fields {};
    for(int i=0;i<12;++i)
    {
        fields[i].automaticPrimary=isAutomaticExpression(proposal.expressions[i]);
        if(!fields[i].automaticPrimary) fields[i].primary=physical(proposal.expressions[i],request.mode)/commaSize(request.mode);
        if(!request.selected[i]&&request.method!=ClosingMethod::allEqually&&!isAutomaticExpression(request.expressions[i]))
            check(request.expressions[i]==proposal.expressions[i],"Fixed formula spelling remains unchanged");
    }
    auto resolved=resolveAutomatic(request.mode,fields);
    auto chart=calculate(request.mode,{std::vector<double>(resolved.fractions.begin(),resolved.fractions.end())});
    check(resolved.valid&&chart.valid,"Proposed formulas actually close");
    check(chart.cents[9]==0,"A stays exactly zero");
    check(std::abs(proposal.closureAfter)<1e-6,"Closure report is accurate");
}
int main()
{
    std::array<std::array<bool,12>,3> seen {};
    std::array<int,12> degree {};
    const auto& notes=cycles(Mode::pythagoreanFifths).front().notes;
    for(const auto& edge:fifthCircleConnections())
    {
        const int kind=static_cast<int>(edge.kind);
        check(!seen[kind][edge.fromIndex],"No duplicate interval on the circle");seen[kind][edge.fromIndex]=true;
        check((notes[edge.toIndex]-notes[edge.fromIndex]+12)%12==harmonySteps(edge.kind),"Circle geometry connects the correct musical interval");
        ++degree[edge.fromIndex];++degree[edge.toIndex];
        int steps=0,index=edge.fromIndex;
        do {index=(index+(edge.toIndex-edge.fromIndex+12)%12)%12;++steps;} while(index!=edge.fromIndex&&steps<13);
        check(steps==(kind==0?12:kind==1?3:4),"Fifths form a ring, major thirds triangles, minor thirds squares");
    }
    for(int count:degree) check(count==6,"Every selected tone has six incident connections");
    for(auto mode:{Mode::pythagoreanFifths,Mode::syntonicFifths})
    {
        for(const auto* input:{"-S/4-H/4","(P-S)/12","1-1/11","-P+11*S/4","ET","2*(P/12-S/6)","(S+P)/(2+2)"})
        {
            CommaComponents c;
            check(expressionComponents(input,mode,c),"Linear expression has symbolic comma coefficients");
            check(std::abs(c.p*pythagoreanComma()+c.s*syntonicComma()-physical(input,mode))<1e-8,"Symbolic coefficients preserve the physical correction");
            check(std::abs(physical(commaExpression(c),mode)-physical(input,mode))<1e-8,"Canonical expression preserves physical correction in either tab");
        }
        ClosingRequest request;request.mode=mode;request.expressions.fill("0");
        for(int edge=0;edge<12;++edge)
        {
            request.selected.fill(false);request.selected[edge]=true;
            const auto closed=proposeClosing(request);verify(request,closed);
            check(closed.expressions[edge]=="-P","Any selected fifth can take the wolf comma");
        }
        request.selected.fill(false);request.selected[2]=request.selected[9]=true;
        auto closed=proposeClosing(request);verify(request,closed);
        check(closed.expressions[2]=="-P/2"&&closed.expressions[9]=="-P/2","Selected fifths split closure equally");
        request.method=ClosingMethod::allEqually;
        closed=proposeClosing(request);verify(request,closed);
        for(const auto& value:closed.expressions) check(value=="-P/12","All-equal closure recovers equal temperament");
        request.method=ClosingMethod::selectedEqually;request.selected.fill(false);request.selected[6]=true;
        request.expressions.fill("-S/4");request.expressions[6]="0";
        closed=proposeClosing(request);verify(request,closed);
        check(std::abs(physical(closed.expressions[6])-(-pythagoreanComma()+11*syntonicComma()/4))<1e-9,"Meantone closing wolf retains exact mixed commas");
        request.expressions.fill("0");request.expressions[0]="-S/4";request.expressions[3]="Auto";request.expressions[8]="Auto";
        request.method=ClosingMethod::automaticFields;
        closed=proposeClosing(request);verify(request,closed);
        check(closed.expressions[3]=="Auto"&&closed.expressions[8]=="Auto","Auto strategy keeps automatic fields persistent");
        check(std::abs(closed.corrections[3]-closed.corrections[8])<1e-9,"Automatic share is equal in physical cents");
        request.method=ClosingMethod::selectedEqually;request.selected.fill(false);request.selected[0]=true;
        closed=proposeClosing(request);verify(request,closed);
        for(const auto& expression:closed.expressions) check(!isAutomaticExpression(expression),"Explicit closure freezes Auto values");
    }
    CommaComponents c;
    check(!expressionComponents("S*S",Mode::pythagoreanFifths,c),"Nonlinear arithmetic is not misidentified as a comma sum");
    check(!expressionComponents("P/(P-S)",Mode::pythagoreanFifths,c),"Division by another comma is nonlinear");
    check(expressionComponents("-1/4-H/4",Mode::syntonicFifths,c)&&commaExpression(c)=="-P/4","Quarter syntonic plus quarter schisma simplifies to quarter P");
    ClosingRequest invalid;invalid.expressions.fill("0");
    check(!proposeClosing(invalid).valid,"An empty selection cannot silently change a fifth");
    invalid.method=ClosingMethod::automaticFields;
    check(!proposeClosing(invalid).valid,"Auto strategy requires automatic fields");
    invalid.method=ClosingMethod::allEqually;invalid.expressions[0]="1/0";
    check(!proposeClosing(invalid).valid,"Bad formulas do not produce a proposal");
    invalid.expressions[0]="S*S";
    const auto nonlinear=proposeClosing(invalid);verify(invalid,nonlinear);
    // Precision mode must improve (or tie) nearest-menu accuracy, retain the
    // imported chart, and emit actual expressions rather than decimal ratios.
    for(const auto* row:{"0;0;0;0;0;0;0;0;0;0;0;0","0;4;-1;2;-3;5;-2;1;3;0;-4;2",
                        "0.322;0.077;-0.095;0.478;-0.478;0.622;-0.390;-0.152;-0.136;-0.392;0.549;-0.252"})
    for(auto mode:{Mode::pythagoreanFifths,Mode::syntonicFifths})
    {
        const auto nearest=reverseCsv(row,mode);
        const auto precise=reverseCsv(row,mode,Reconstruction::precise);
        check(precise.valid&&precise.cents==nearest.cents,"Precise reconstruction preserves CSV and A reference");
        const auto& notes=cycles(mode).front().notes;
        double closure=pythagoreanComma();
        for(int i=0;i<12;++i)
        {
            check(std::abs(precise.intervalErrors[i])<=std::max(0.00100001,std::abs(nearest.intervalErrors[i]))+1e-8,"Readable precision respects CSV uncertainty or improves nearest-menu accuracy");
            check(precise.expressions[i].find('.')==std::string::npos,"Precise output uses fractional expressions");
            const double correction=physical(precise.expressions[i],mode);
            const double target=700+precise.cents[notes[(i+1)%12]]-precise.cents[notes[i]]-pureInterval(mode);
            check(std::abs(correction-target-precise.intervalErrors[i])<1e-8,"Reported error includes every comma term");
            closure+=correction;
        }
        check(std::abs(closure-precise.closureError)<1e-8,"Precision closure report matches its formulas");
    }
    // Exact two-term targets are discoverable, in both comma representations.
    for(const auto* expression:{"-P/24","S/7-P/11","-P+11*S/4","-H/13+S/17"})
    {
        const auto reconstructed=preciseExpression(physical(expression),Mode::syntonicFifths);
        check(std::abs(physical(reconstructed.text)-physical(expression))<=0.00100001,"Readable reconstruction is sufficient at CSV precision");
    }
    // A closed fractional tuning reconstructed from rounded cents: infer all
    // fifths together rather than forcing the independent nearest choices.
    for(int tuning=0;tuning<3;++tuning)
    {
        ClosingRequest original;original.expressions.fill(tuning==0?"-P/12":tuning==1?"-S/4":"0");
        if(tuning==1) original.expressions[6]="-P+11*S/4";
        if(tuning==2) for(int i=0;i<4;++i) original.expressions[i]="-P/4";
        std::vector<double> values;for(const auto& expression:original.expressions) values.push_back(physical(expression)/pythagoreanComma());
        const auto chart=calculate(Mode::pythagoreanFifths,{values});check(chart.valid,"Inference fixture closes");
        const auto imported=reverseCsv(csv(chart.cents),Mode::syntonicFifths);
        ClosingRequest fit;fit.mode=Mode::syntonicFifths;fit.method=ClosingMethod::simpleFit;fit.expressions=imported.expressions;
        fit.selected.fill(true);fit.useTargets=true;fit.maximumChange=0.01;
        const auto& notes=cycles(fit.mode).front().notes;
        for(int i=0;i<12;++i) fit.targets[i]=700+imported.cents[notes[(i+1)%12]]-imported.cents[notes[i]]-pureInterval(fit.mode);
        auto proposal=proposeClosing(fit);verify(fit,proposal);
        for(int i=0;i<12;++i)
        {
            check(std::abs(proposal.changes[i])<=fit.maximumChange+1e-8,"Inferred fifth respects the requested limit");
            check(std::abs(proposal.corrections[i]-physical(original.expressions[i]))<1e-7,"Inference recovers the simple known temperament");
        }
        if(tuning==1)
        {
            fit.selected.fill(false);fit.selected[6]=true;
            // Restore fixed meantone fractions, then infer only the wolf.
            fit.expressions=original.expressions;
            proposal=proposeClosing(fit);verify(fit,proposal);
        }
    }
    ClosingRequest impossible;impossible.expressions.fill("0");impossible.selected.fill(true);
    impossible.method=ClosingMethod::simpleFit;impossible.maximumChange=0.001;
    check(!proposeClosing(impossible).valid,"Inference honestly rejects an impossible change limit");
    impossible.maximumChange=2;
    verify(impossible,proposeClosing(impossible));
    impossible.maximumChange=std::numeric_limits<double>::quiet_NaN();
    check(!proposeClosing(impossible).valid,"Inference rejects invalid limits");
    const std::string meantoneCsv="10.265;-13.686;3.422;20.530;-3.421;13.686;-10.265;6.843;-17.108;0.000;17.108;-6.843";
    for(auto mode:{Mode::pythagoreanFifths,Mode::syntonicFifths})
    {
        const auto recovered=reverseCsv(meantoneCsv,mode,Reconstruction::precise);
        for(int i=0;i<12;++i)
            if(i!=8) check(recovered.expressions[i]=="-S/4","User's meantone keeps eleven simple quarter-syntonic fifths");
        check(std::abs(physical(recovered.expressions[8])-(11*syntonicComma()/4-pythagoreanComma()))<1e-8,"Wolf carries the exact remaining comma correction");
        check(std::abs(recovered.closureError)<1e-6,"User's meantone reconstruction closes");
        std::vector<double> corrections(12,0);
        corrections[0]=0.75*syntonicComma()/commaSize(mode);
        corrections[6]=closingFraction(mode,corrections,6);
        const auto multiples=reverseCsv(csv(calculate(mode,{corrections}).cents),mode);
        check(multiples.expressions[0]=="S*3/4","Nearest mode writes low multiples with the comma name first");
        check(std::abs(physical(multiples.expressions[0])-0.75*syntonicComma())<1e-8,"Small multiple retains its physical comma unit");
    }
    const auto rosales=reverseCsv("0.322;0.077;-0.095;0.478;-0.478;0.622;-0.390;-0.152;-0.136;-0.392;0.549;-0.252",Mode::syntonicFifths,Reconstruction::precise);
    ClosingRequest rosalesFit;rosalesFit.mode=Mode::syntonicFifths;rosalesFit.expressions=rosales.expressions;
    rosalesFit.method=ClosingMethod::simpleFit;rosalesFit.selected.fill(true);rosalesFit.useTargets=true;
    for(int i=0;i<12;++i)
    {
        rosalesFit.targets[i]=700+rosales.cents[notes[(i+1)%12]]-rosales.cents[notes[i]]-pureInterval(rosalesFit.mode);
        check(std::abs(rosales.intervalErrors[i])<=0.00100001,"User's measured CSV gets expressions within rounding precision");
        int number=0;
        for(char ch:rosales.expressions[i])
        {
            if(ch>='0'&&ch<='9') {number=number*10+ch-'0';check(number<=24,"Readable reconstruction never hides giant cancelling numbers");}
            else number=0;
        }
    }
    const auto rosalesClosed=proposeClosing(rosalesFit);verify(rosalesFit,rosalesClosed);
    for(const auto& expression:rosalesClosed.expressions) check(expression.size()<60,"Measured-circle closure uses short small-number expressions");
    check(rosalesClosed.maximumChange<=0.1+1e-8,"Measured-circle closure respects the chosen change limit");
    const auto& known=knownTemperaments();
    check(known.size()>=31,"All supplied catalogue rows are embedded");
    for(size_t i=0;i<known.size();++i)
    {
        const auto matches=recogniseTemperament(known[i].cents);
        check(std::any_of(matches.begin(),matches.end(),[i](const auto& m){return m.index==i&&m.maximumError<1e-8&&m.rotationFifths==0;}),"Catalogue recognises every exact entry including aliases");
        auto shifted=known[i].cents;for(auto& value:shifted) value+=8.125;
        shifted[0]+=0.04;
        const auto nearMatches=recogniseTemperament(shifted);
        check(std::any_of(nearMatches.begin(),nearMatches.end(),[i](const auto& m){return m.index==i&&m.maximumError<=0.040001;}),"Catalogue recognises nearby tuning after A normalisation");
    }
    const auto meantoneMatch=recogniseTemperament(reverseCsv(meantoneCsv).cents);
    check(!meantoneMatch.empty()&&known[meantoneMatch.front().index].name=="Meantone (-1/4)","User's meantone is named from the embedded catalogue");
    const auto originalMeantone=reverseCsv(meantoneCsv).cents;
    std::array<double,12> rotated;
    for(int i=0;i<12;++i) rotated[i]=originalMeantone[(i-3+12)%12];
    const auto rotatedMatches=recogniseTemperament(rotated);
    check(!rotatedMatches.empty()&&known[rotatedMatches.front().index].name=="Meantone (-1/4)"&&rotatedMatches.front().rotationFifths==9,"Three semitones correspond to three fifths counter-clockwise");
    const auto measuredSuggestions=recogniseTemperament(rosales.cents);
    check(measuredSuggestions.size()>=5&&std::all_of(measuredSuggestions.begin(),measuredSuggestions.end(),[](const auto& m){return m.maximumError>1;}),
        "Measured tuning receives comparisons without inventing a close identity");
    check(rotationDescription(1)=="1 fifth clockwise"&&rotationDescription(-1)=="1 fifth counter-clockwise","Rotation descriptions follow the circle buttons");
    const auto aliases=recogniseTemperament(std::array<double,12>{});
    check(aliases.size()>=5&&known[aliases[0].index].name=="Equal"&&known[aliases[1].index].name=="Neidhardt Hof (=Equal)","Exact aliases come before harmonic alternatives");
    check(std::count_if(aliases.begin(),aliases.end(),[](const auto& m){return m.maximumError<=1;})==2,
        "Identical rotations of equal temperament do not clutter the suggestions");
    const auto rousseau=reverseCsv("10.26;-8.31;3.42;-2.2;-3.42;8.31;-10.26;6.84;-6.35;0;4.15;-6.84").cents;
    for(int rotation=0;rotation<12;++rotation)
    {
        auto variant=rotateChartFifths(rousseau,rotation);
        for(auto& value:variant) value+=123.45;
        const auto suggestions=recogniseTemperament(variant);
        check(suggestions.size()>=5,"Distant variant gets several distinct suggestions in every rotation");
        const auto ordinaire=std::find_if(suggestions.begin(),suggestions.end(),[&](const auto& m){return known[m.index].name=="Ordinaire";});
        check(ordinaire!=suggestions.end(),"User's Rousseau variant suggests Ordinaire in every rotation and reference");
        if(ordinaire!=suggestions.end())
        {
            check(ordinaire->rotationFifths==rotation&&ordinaire->maximumError>1,"Ordinaire is a rotated comparison, not an asserted identity");
            check(std::abs(ordinaire->similarityError-2.699915122122669)<1e-9,"Harmonic distance is invariant under rotation and reference shifts");
            check(std::abs(ordinaire->fifthRmsError-1.8688811269491346)<1e-9,"Fifth pattern of the Rousseau variant is compared across the complete circle");
        }
        for(size_t i=1;i<suggestions.size();++i)
        {
            check(suggestions[i-1].similarityError<=suggestions[i].similarityError,"Alternatives are ranked by increasing harmonic distance");
            for(size_t j=0;j<i;++j) check(suggestions[i].index!=suggestions[j].index,"Best rotations do not crowd out other catalogue names");
        }
    }
    std::array<double,12> extreme {};extreme[0]=10000;
    const auto distant=recogniseTemperament(extreme);
    check(distant.size()>=5&&distant.front().maximumError>1000,"Even an unrelated tuning gets explicitly distant comparisons with honest errors");
    extreme[0]=std::numeric_limits<double>::infinity();
    check(recogniseTemperament(extreme).empty(),"Nonfinite charts cannot produce catalogue suggestions");
    check(recogniseTemperament(rousseau,-1).empty(),"Invalid near-match tolerances are rejected");
    for(auto from:{Mode::pythagoreanFifths,Mode::syntonicFifths})
    {
        const auto to=from==Mode::pythagoreanFifths?Mode::syntonicFifths:Mode::pythagoreanFifths;
        for(const std::string expression:{"-1/4","1-1/11","-S/4-schisma/4","P*3/5","3*P/5","1-P/11", "(P+S)/(1+S)","0.123456789123"})
        {
            const auto converted=expressionInMode(expression,from,to);
            check(!converted.empty()&&std::abs(physical(expression,from)-physical(converted,to))<1e-9,"Tab conversion preserves mixed, bare, decimal and nonlinear expressions in cents");
            check(std::abs(physical(expressionInMode(converted,to,from),from)-physical(expression,from))<1e-9,"Returning to the original unit does not quantise the tuning");
        }
        check(expressionInMode("Auto",from,to)=="Auto","Tab conversion retains Auto");
        check(expressionInMode("1/0",from,to).empty(),"Invalid fields clear the other tab instead of displaying old tuning");
        check(commaExpression({0.6,0})=="P*3/5","Generated rational formulas put the comma first");
    }
    for(size_t index=0;index<known.size();++index)
    {
        for(int step=0;step<12;++step)
        {
            const auto shifted=rotateChartFifths(known[index].cents,step);
            check(shifted[9]==0,"Every rotated chart keeps A exactly zero");
            const auto restored=rotateChartFifths(shifted,-step);
            for(int i=0;i<12;++i) check(std::abs(restored[i]-known[index].cents[i])<1e-10,"Opposite rotations restore all note deviations");
            const auto matches=recogniseTemperament(shifted);
            check(std::any_of(matches.begin(),matches.end(),[index](const auto& m){return m.index==index&&m.maximumError<1e-8;}),"All catalogue entries are recognised in all fifth rotations");
        }
        if(known[index].name.find("Trost")==std::string::npos) continue;
        for(auto mode:{Mode::pythagoreanFifths,Mode::syntonicFifths})
        for(auto method:{Reconstruction::nearestFraction,Reconstruction::precise})
        {
            const auto trost=reverseCsv(csv(known[index].cents),mode,method);
            for(int pivot=0;pivot<12;++pivot)
            {
                std::array<FifthCorrection,12> fields {};
                for(int i=0;i<12;++i) fields[i].primary=physical(trost.expressions[i],mode)/commaSize(mode);
                fields[pivot].automaticPrimary=true;
                const auto automatic=resolveAutomatic(mode,fields);
                const auto chart=calculate(mode,{std::vector<double>(automatic.fractions.begin(),automatic.fractions.end())});
                check(chart.valid,"Trost closes with any one fifth set to Auto");
                for(int step=0;step<12;++step)
                {
                    const auto matches=recogniseTemperament(rotateChartFifths(chart.cents,step));
                    check(std::any_of(matches.begin(),matches.end(),[index,step](const auto& m){return m.index==index&&m.rotationFifths==step;}),"Trost remains a nearby catalogue match after nearest-fraction import, Auto closure and rotation");
                }
            }
        }
    }
    std::cout<<checks<<" reconstruction and closure checks passed\n";
}
