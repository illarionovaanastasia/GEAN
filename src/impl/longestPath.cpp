//
// Created by Baoxing Song on 2019-03-13.
//

#include "longestPath.h"



/**
 * this function try to keep those genes in the syntenic region using a longest path algorithm
 * which is a kind of global alignment method
 *
 * longest path algorithm from here:
 * https://www.geeksforgeeks.org/find-longest-path-directed-acyclic-graph/
 *
 */


void myPOrthologPairsSort( std::vector<OrthologPair> & pairedSimilarFragments, const double & score, const double & penalty, const double & scoreThreshold, const bool & keepTandemDuplication){
    std::sort(pairedSimilarFragments.begin(), pairedSimilarFragments.end(), [](OrthologPair a, OrthologPair b) {
        return a < b;
    });

    int startIndex=0;
    int endIndex=0;
    double maxScore=0;
    double currentScore=0;
    for( int idx=0;  idx<pairedSimilarFragments.size(); ++idx){
        if( NEGATIVE == pairedSimilarFragments[idx].getStrand() ){  // reverse strand  we need to do something special for inversion
            // look for all following pairs that are reverse strand,
            // ie get all reverse strand entries in this group of reverse strands
            for( int jdx=idx; jdx< pairedSimilarFragments.size(); ++jdx ){
                if( NEGATIVE == pairedSimilarFragments[jdx].getStrand() ) {
                    if( idx == jdx ){ // the first one.
                        currentScore+=score*pairedSimilarFragments[jdx].getScore();
                    }else{ // for the reverse alignments, check the strand of the previous alignment
                        // If both current and previous are reverse strand, and
                        // if previous assembly start is greater than current assembly start, increase score.
                        //  else, apply penalty (they are out of order)
                        if ( pairedSimilarFragments[jdx-1].getQueryMiddlePos() > pairedSimilarFragments[jdx].getQueryMiddlePos() ){ // GOOD INVERSION
                            currentScore+=score*pairedSimilarFragments[jdx].getScore();
                        }else if(keepTandemDuplication && pairedSimilarFragments[jdx-1].getRefMiddlePos() == pairedSimilarFragments[jdx].getRefMiddlePos() ){ // tandem duplication
                            currentScore+=score*pairedSimilarFragments[jdx].getScore();
                        }else{
                            currentScore+=penalty*pairedSimilarFragments[jdx].getScore(); // GIVE PENALTY
                        }
                    }
                }else{
                    currentScore+=penalty*pairedSimilarFragments[jdx].getScore(); // penalty because are now forward strand
                }
                if (maxScore < currentScore) {
                    maxScore = currentScore;
                    endIndex=jdx; // keeps track of where to stop the reverse strand grouping
                }
                // If score is negative, stop the loop.  This will happen as we find more
                // forward vs reverse strands.  If there was just 1 reverse strand followed
                // by a forward strand, currentScore goes from 3 to -1 (3 plus -4)
                if( currentScore<0 ){
                    break;
                }
            }
            // if maxScore is larger than scoreThreshold, it means there were multiple reverse
            // strand entries in this group, or a single reverse alignment of significant length.
            // If we have several reverse alignments, we think it is real.  If just 1, may be false
            // alignment.  If the maxScore is greater than the scoreThreshold, we treat it as real.
            // Flip all elements in the range so assembly coordinates are in increasing order.
            if( maxScore>scoreThreshold ){
                maxScore = 0.0 ;
                currentScore = 0.0;
                // loop to find start index of elements we want to flip
                for( int jdx=endIndex; jdx>=idx; --jdx ){
                    if( NEGATIVE == pairedSimilarFragments[jdx].getStrand() ) {
                        if( jdx>idx ){
                            // Verify the reverse alignments are in order to each other.  If not,
                            // apply penalty.  This doesn't prevent overlaps, which will be dealt with later.
                            if ( pairedSimilarFragments[jdx-1].getQueryMiddlePos() > pairedSimilarFragments[jdx].getQueryMiddlePos() ){
                                currentScore+=score*pairedSimilarFragments[jdx].getScore();
                            }else if( keepTandemDuplication && pairedSimilarFragments[jdx-1].getRefMiddlePos() == pairedSimilarFragments[jdx].getRefMiddlePos() ){ // tandem duplication
                                currentScore+=score*pairedSimilarFragments[jdx].getScore();
                            }else{
                                currentScore+=penalty*pairedSimilarFragments[jdx].getScore(); // GIVE PENALTY
                            }
                        }else{
                            currentScore+=score*pairedSimilarFragments[jdx].getScore();
                        }
                    }else{
                        currentScore+=penalty*pairedSimilarFragments[jdx].getScore(); // GIVE PENALTY
                    }
                    if (maxScore < currentScore) {
                        maxScore = currentScore;
                        startIndex=jdx;
                    }
                    if( currentScore<0 ){
                        break;
                    }
                }

                int length = (endIndex-startIndex+1)/2;
                if(length > 0) {
                    for (int j = 0; j < length; ++j) {
                        OrthologPair temp = pairedSimilarFragments[startIndex + j];
                        pairedSimilarFragments[startIndex + j]=pairedSimilarFragments[endIndex - j];
                        pairedSimilarFragments[endIndex - j] = temp;
                    }
                    // Flip the elements in the list so the asm reverse alignments
                    // all have increasing asm values. (ie the last element of the reverse
                    // grouping is now the first, and the first is the last.
                    bool thereAreReverseAlignments = true;
                    while (thereAreReverseAlignments) {
                        thereAreReverseAlignments = false;
                        for (int j = 1; j < length; ++j) {
                            // If a single reference position has multiple assembly alignments mapping to it,
                            // swap the order until the assembly positions are all increasing.
                            if (pairedSimilarFragments[startIndex + j - 1].getRefMiddlePos() == pairedSimilarFragments[startIndex + j].getRefMiddlePos() &&
                                pairedSimilarFragments[startIndex + j - 1].getQueryMiddlePos() > pairedSimilarFragments[startIndex + j].getQueryMiddlePos() ) {
                                thereAreReverseAlignments = true;
                                OrthologPair temp = pairedSimilarFragments[startIndex + j];
                                pairedSimilarFragments[startIndex + j]=pairedSimilarFragments[startIndex + j - 1];
                                pairedSimilarFragments[startIndex + j - 1]=temp;
                            }
                        }
                    }
                }
                // Flip the elements in the list so the asm reverse ailgnments
                // all have increasing asm values. (ie the last element of the reverse
                // grouping is now the first, and the first is the last.

                idx=endIndex;
            }
            maxScore=0.0;
            currentScore=0.0;
        }
    }
}



void longestPath (std::vector<OrthologPair> & pairedSimilarFragments, std::vector<OrthologPair> & sortedOrthologPairs, const bool & keepTandemDuplication){
    double maxSore = 0;
    int bestEnd = 0;
    double scoreArray [pairedSimilarFragments.size()]; // arrays of scores
    int prev [pairedSimilarFragments.size()];  // index of previous node in longest path
    scoreArray[0] = pairedSimilarFragments[0].getScore();
    prev[0] = -1;
    for (int idx = 1; idx < pairedSimilarFragments.size(); ++idx) {
        scoreArray[idx] = pairedSimilarFragments[idx].getScore();
        prev[idx] = -1;
        for (int jdx = idx - 1; jdx >= 0; --jdx) {// checking all previous nodes
            // Because we swapped asm/query start position so that inversions were all increasing,
            // we should always be on the diagonal.  If not, then we filter it.
            // This gets rid of the noise, while preserving the inversions on
            // the diagonal
            // Are only looking at positions previous to our current "idx" position
            if ( (scoreArray[jdx] + pairedSimilarFragments[idx].getScore()) > scoreArray[idx] &&
                pairedSimilarFragments[jdx].getQueryMiddlePos() < pairedSimilarFragments[idx].getQueryMiddlePos()){
                scoreArray[idx] = scoreArray[jdx] + pairedSimilarFragments[idx].getScore();
                prev[idx] = jdx;
            }else if ( (scoreArray[jdx] + pairedSimilarFragments[idx].getScore()) > scoreArray[idx] &&
                      pairedSimilarFragments[jdx].getQueryMiddlePos() == pairedSimilarFragments[idx].getQueryMiddlePos()
                      && keepTandemDuplication) {
                scoreArray[idx] = scoreArray[jdx] + pairedSimilarFragments[idx].getScore();
                prev[idx] = jdx;
            }
        }
        if (scoreArray[idx] > maxSore){
            bestEnd = idx;
            maxSore = scoreArray[idx];
        }
    }
    int idx=bestEnd; // bestEnd is where to stop the longest path
    sortedOrthologPairs.push_back(pairedSimilarFragments[idx]);
    int jdx = prev[idx]; // prev[] is index on the longest path
    while( jdx>=0 ){
        sortedOrthologPairs.push_back(pairedSimilarFragments[jdx]);
        jdx=prev[jdx];
    }
    // Reversing the order
    std::reverse(std::begin(sortedOrthologPairs), std::end(sortedOrthologPairs));
}


void outputGffRecords(Gene & gene, std::ofstream &ofile, std::set<std::string> & geneNames, std::set<std::string> & transcriptNames  ){
    std::string newGeneName=gene.getName();
    if( geneNames.find(newGeneName) != geneNames.end() ){
        int i=1;
        newGeneName = newGeneName + "_" + std::to_string(i);
        while( geneNames.find(newGeneName) != geneNames.end() ){
            ++i;
            newGeneName = gene.getName() + "_" + std::to_string(i);
        }
        geneNames.insert(newGeneName);
    }else{
        geneNames.insert(newGeneName);
    }
    if( gene.getTranscripts().size() > 0 ){
        for(  std::vector<Transcript>::iterator it=gene.getTranscripts().begin();
              it!=gene.getTranscripts().end(); ++it){
            int thisTranscriptStart=std::numeric_limits<int>::max();
            int thisTranscriptEnd=0;
            if( it->getExonVector().size() > 0 ) {
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getExonVector().begin();
                     it4 != it->getExonVector().end(); ++it4) {
                    if(thisTranscriptStart>it4->getStart()){
                        thisTranscriptStart=it4->getStart();
                    }
                    if(thisTranscriptEnd<it4->getStart()){
                        thisTranscriptEnd=it4->getEnd();
                    }
                }
            }
            if( it->getCdsVector().size() > 0 ) {
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getCdsVector().begin();
                     it4 != it->getCdsVector().end(); ++it4) {
                    if(thisTranscriptStart>it4->getStart()){
                        thisTranscriptStart=it4->getStart();
                    }
                    if(thisTranscriptEnd<it4->getStart()){
                        thisTranscriptEnd=it4->getEnd();
                    }
                }
            }
            it->setStart(thisTranscriptStart);
            it->setEnd(thisTranscriptEnd);
        }
        int thisStart = gene.getTranscripts()[0].getStart();
        int thisEnd = gene.getTranscripts()[0].getEnd();
        for(  std::vector<Transcript>::iterator it=gene.getTranscripts().begin();
              it!=gene.getTranscripts().end(); ++it){
            if( thisStart> it->getStart() ){
                thisStart = it->getStart();
            }
            if( thisEnd> it->getEnd() ){
                thisEnd = it->getEnd();
            }
        }
        std::string st = "+";
        if( NEGATIVE ==  gene.getTranscripts()[0].getStrand() ){
            st="-";
        }
        std::string transcriptResource = gene.getTranscripts()[0].getSource();
        if( transcriptResource.length()<1 ){
            transcriptResource="LIFTOVER";
        }
        ofile << gene.getTranscripts()[0].getChromeSomeName() << "\t"+transcriptResource+"\tgene\t" << thisStart << "\t" <<
              thisEnd << "\t.\t"<< st <<"\t.\tID=" << newGeneName<<";" << std::endl;
        for( std::vector<Transcript>::iterator it=gene.getTranscripts().begin();
             it!=gene.getTranscripts().end(); ++it ){
            transcriptResource = it->getSource();
            if( transcriptResource.length()<1 ){
                transcriptResource="LIFTOVER";
            }

            std::string newTranscriptName=it->getName();
            if( transcriptNames.find(newTranscriptName) != transcriptNames.end() ){
                int i=1;
                newTranscriptName = newTranscriptName + "_" + std::to_string(i);
                while( transcriptNames.find(newTranscriptName) != transcriptNames.end() ){
                    ++i;
                    newTranscriptName = it->getName() + "_" + std::to_string(i);
                }
                transcriptNames.insert(newTranscriptName);
            }else{
                transcriptNames.insert(newTranscriptName);
            }

            ofile << it->getChromeSomeName() << "\t"+transcriptResource+"\t"<< it->getType() <<"\t" << it->getStart() << "\t" <<
                  it->getEnd() << "\t" << it->getScore() << "\t"<< st <<"\t.\tID="<< newTranscriptName<<";Parent=" << newGeneName<<";" << std::endl;
            std::vector<GenomeBasicFeature> GenomeBasicFeatures;
            if( it->getFivePrimerUtr().size()>0){
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getFivePrimerUtr().begin();
                     it4 != it->getFivePrimerUtr().end(); ++it4) {
                    GenomeBasicFeatures.push_back(*it4);
                }
            }
            if( it->getCdsVector().size() > 0 ) {
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getCdsVector().begin();
                     it4 != it->getCdsVector().end(); ++it4) {
                    GenomeBasicFeatures.push_back(*it4);
                }
            }
            if( it->getExonVector().size() > 0 ) {
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getExonVector().begin();
                     it4 != it->getExonVector().end(); ++it4) {
                    GenomeBasicFeatures.push_back(*it4);
                }
            }
            if( it->getThreePrimerUtr().size()>0){
                for (std::vector<GenomeBasicFeature>::iterator it4 = it->getThreePrimerUtr().begin();
                     it4 != it->getThreePrimerUtr().end(); ++it4) {
                    GenomeBasicFeatures.push_back(*it4);
                }
            }
            if( GenomeBasicFeatures.size() > 0 ) {
                std::sort(GenomeBasicFeatures.begin(), GenomeBasicFeatures.end(), [](GenomeBasicFeature a, GenomeBasicFeature b) {
                    return a.getStart() < b.getStart();
                });
                for (std::vector<GenomeBasicFeature>::iterator it4 = GenomeBasicFeatures.begin();
                     it4 != GenomeBasicFeatures.end(); ++it4) {
                    ofile << it->getChromeSomeName() << "\t" + transcriptResource + "\t"+(*it4).getType()+"\t" << (*it4).getStart() << "\t"
                          << (*it4).getEnd() << "\t" << it->getScore() << "\t" << st << "\t" << it4->getCodonFrame() << "\tParent="
                          << newTranscriptName << ";" << std::endl;
                }
            }
            if( it->getCdsVector().size() > 0 ){
                ofile << "#metainformation: " << it->getMetaInformation() << std::endl;
                ofile << "#genome sequence: " << it->getGeneomeSequence() << std::endl;
                ofile << "#CDS sequence: " << it->getCdsSequence() << std::endl;
            }
            std::string cdsSequence = it->getCdsSequence();
        }
    }
}

void generateLongestOutput( const std::string & referenceGffFile, const std::string & queryNewGffFile,
                                const std::string & queryGenomeFile, const std::string & outputGffFile, const int & minIntron,
                                double score, double penalty, double scoreThreshold, const bool & keepTandemDuplication,
                                std::map<std::string, std::string>& parameters, const double & syntenicScore, const double & orfScore, const double & dropLengthThredshold ){
//    std::cout << "longest line 308" << std::endl;
    std::map<std::string, Fasta> queryGenome;
    readFastaFile(queryGenomeFile, queryGenome);
//    std::cout << "longest line 311" << std::endl;
    NucleotideCodeSubstitutionMatrix nucleotideCodeSubstitutionMatrix(parameters);
//    std::cout << "longest line 313" << std::endl;
    std::map<std::string, std::vector<Gene> > queryGenes;
//    std::cout << "longest line 315" << std::endl;
    readGffFileWithEveryThing (queryNewGffFile, queryGenes);
//    std::cout << "longest line 317" << std::endl;
    std::map<std::string, std::vector<std::string> > referenceGeneNameMap;
    std::map<std::string, Gene > referenceGeneHashMap;
    std::map<std::string, Transcript> referenceTranscriptHashMap;
//    std::cout << "longest line 321" << std::endl;
    readGffFileWithEveryThing ( referenceGffFile, referenceGeneNameMap, referenceGeneHashMap, referenceTranscriptHashMap);
//    std::cout << "longest line 323" << std::endl;
    std::ofstream ofile;
    ofile.open(outputGffFile);
    std::set<std::string> geneNames;
    std::set<std::string> transcriptNames;
//    std::cout << "longest line 328" << std::endl;

    std::map<std::string, Gene> keepGenes;
    std::map<std::string, double> geneScores;

    for(std::map<std::string,std::vector<Gene>>::iterator it1=queryGenes.begin(); it1!=queryGenes.end(); it1++){
        std::vector<OrthologPair> orthologPairs;
        if( referenceGeneNameMap.find(it1->first) != referenceGeneNameMap.end() && queryGenome.find(it1->first)!=queryGenome.end() ){
            for( int i=1; i< it1->second.size(); ++i ){
                if( referenceGeneHashMap.find(it1->second[i].getName())!=referenceGeneHashMap.end()
                    && referenceGeneHashMap[it1->second[i].getName()].getChromeSomeName().compare(it1->second[i].getChromeSomeName())==0  ){
                    int targetIndex=i;
                    uint32_t refStartPos=referenceGeneHashMap[it1->second[i].getName()].getStart();
                    uint32_t refEndPos=referenceGeneHashMap[it1->second[i].getName()].getEnd();
                    uint32_t queryStartPos=it1->second[i].getStart();
                    uint32_t queryEndPos=it1->second[i].getEnd();
                    double thisScore=0;

                    for(  int index=0; index<it1->second[i].getTranscripts().size(); ++index ){
                        Transcript transcript=it1->second[i].getTranscripts()[index];
                        TranscriptUpdateCdsInformation(transcript, queryGenome);
                        checkOrfState(transcript, queryGenome, nucleotideCodeSubstitutionMatrix, minIntron);
                        if( transcript.getIfOrfShift() ){
                            // this score is for gene, if there is one ORF conserved transcript, then give this gene a positive score
                        }else{
                            thisScore=orfScore;
                        }
                        it1->second[i].getTranscripts()[index]=transcript;
                        //break;
                    }

                    double lengthRatio;
                    if( it1->second[i].getEnd() == it1->second[i].getStart() ){
                        lengthRatio=0.0;
                    }else{
                        lengthRatio=(double)((double)it1->second[i].getEnd()-(double)it1->second[i].getStart())/((double)referenceGeneHashMap[it1->second[i].getName()].getEnd()-(double)referenceGeneHashMap[it1->second[i].getName()].getStart());
                    }
                    if( lengthRatio > 1 ){
                        lengthRatio=1/lengthRatio;
                    }
                    thisScore += lengthRatio;
                    STRAND strand;
                    if( it1->second[i].getStrand() == referenceGeneHashMap[it1->second[i].getName()].getStrand() ){
                        strand=POSITIVE;
                    }else{
                        strand=NEGATIVE;
                    }
                    if( lengthRatio > dropLengthThredshold ){
                        OrthologPair orthologPair( targetIndex, refStartPos, refEndPos, queryStartPos, queryEndPos, thisScore, strand );
                        orthologPairs.push_back(orthologPair);
                    }
                }
            }
            myPOrthologPairsSort( orthologPairs, score, penalty,  scoreThreshold, keepTandemDuplication);
            std::vector<OrthologPair> sortedOrthologPairs;
            longestPath (orthologPairs, sortedOrthologPairs, keepTandemDuplication);
            for( OrthologPair orthologPair : sortedOrthologPairs ){
                //outputGffRecords(it1->second[orthologPair.getQueryIndex()], ofile, geneNames, transcriptNames  );
                Gene g = it1->second[orthologPair.getQueryIndex()];
                double thisScore = orthologPair.getScore() + syntenicScore;
                if( geneScores.find(g.getName()) != geneScores.end() ){
                    if( geneScores[g.getName()] < thisScore ){
                        geneScores[g.getName()] = thisScore;
                        keepGenes[g.getName()] = g;
                    }
                }else{
                    geneScores[g.getName()] = thisScore;
                    keepGenes[g.getName()] = g;
                }
            }
        }
    }

    //std::cout << " line 385" << std::endl;
    for(std::map<std::string,std::vector<Gene>>::iterator it1=queryGenes.begin(); it1!=queryGenes.end(); it1++){
        for( Gene g : it1->second ){
            //if( geneNames.find(g.getName())!=geneNames.end() ){

            //}else{
                double thisScore=0;
                for(  int index=0; index<g.getTranscripts().size(); ++index ){
                    Transcript transcript=g.getTranscripts()[index];
                    TranscriptUpdateCdsInformation(transcript, queryGenome);
                    checkOrfState(transcript, queryGenome, nucleotideCodeSubstitutionMatrix, minIntron);
                    if( transcript.getIfOrfShift() ){
                        // this score is for gene, if there is one ORF conserved transcript, then give this gene a positive score
                    }else{
                        thisScore=orfScore;
                    }
                    g.getTranscripts()[index]=transcript;
                }

                double lengthRatio;
                if( g.getEnd() == g.getStart() ){
                    lengthRatio=0.0;
                }else{
                    lengthRatio=(double)((double)g.getEnd()-(double)g.getStart())/((double)referenceGeneHashMap[g.getName()].getEnd()-(double)referenceGeneHashMap[g.getName()].getStart());
                }
                if( lengthRatio > 1 ){
                    lengthRatio=1/lengthRatio;
                }
                thisScore += lengthRatio;
                if( geneScores.find(g.getName()) != geneScores.end() ){
                    if( geneScores[g.getName()] < thisScore ){
                        geneScores[g.getName()] = thisScore;
                        keepGenes[g.getName()] = g;
                    }
                }else{
                    geneScores[g.getName()] = thisScore;
                    keepGenes[g.getName()] = g;
                }
            //}
        }
    }
    for( std::map<std::string, Gene>::iterator it = keepGenes.begin(); it!=keepGenes.end(); ++ it ){
        outputGffRecords(it->second, ofile, geneNames, transcriptNames  );
    }
    ofile.close();
}
