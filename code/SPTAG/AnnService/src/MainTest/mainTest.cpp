// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Helper/VectorSetReader.h"
#include "inc/Core/VectorIndex.h"
#include "inc/Core/Common.h"
#include "inc/Helper/SimpleIniReader.h"

#include <memory>
#include <inc/Core/Common/DistanceUtils.h>

#include "inc/Helper/CommonHelper.h"
#include "inc/Helper/StringConvert.h"
#include "inc/Helper/AsyncFileReader.h"
#include "inc/Core/Common/CommonUtils.h"
#include "inc/Core/Common/TruthSet.h"
#include "inc/Core/Common/QueryResultSet.h"
#include <algorithm>
#include <iomanip>
#include <set>
#include <atomic>
#include <ctime>
#include <thread>
#include <chrono>
#include <unistd.h>

using namespace SPTAG;

class Options : public Helper::ReaderOptions
{
public:
    Options() : Helper::ReaderOptions(VectorValueType::Float, 0, VectorFileType::DEFAULT, "|", 32)
    {
        AddOptionalOption(m_outputFolder, "-ip", "--index-path", "Index path.");
        AddOptionalOption(m_indexAlgoType, "-a", "--algorithm", "Index Algorithm type.");
        AddOptionalOption(m_inputFiles, "-data", "--dataset", "input path.");
        AddOptionalOption(m_builderConfigFile, "-c", "--config", "Config file for builder.");
        AddOptionalOption(m_quantizerFile, "-pq", "--quantizer", "Quantizer File");
        AddRequiredOption(m_mode, "-m", "--mode", " 0 : build | 1 : search");
        AddOptionalOption(m_metaMapping, "-mi", "--metaindex", "Enable delete vectors through metadata");
        AddOptionalOption(num_points, "-ds", "--dataset-size", "Dataset size");

        AddOptionalOption(num_query, "-qs", "--queries-size", "Queries size");
        AddOptionalOption(m_queryFile, "-q", "--queries", "Queries path.");
        AddOptionalOption(m_indexFolder, "-x", "--index", "Index folder.");
        AddOptionalOption(m_truthFile, "-r", "--truth", "Truth file.");
        AddOptionalOption(m_resultFile, "-o", "--result", "Output result file.");
        AddOptionalOption(m_maxCheck, "-mc", "--maxcheck", "MaxCheck for index.");
        AddOptionalOption(m_withMeta, "-a", "--withmeta", "Output metadata instead of vector id.");
        AddOptionalOption(m_K, "-k", "--KNN", "K nearest neighbors for search.");
        AddOptionalOption(m_truthK, "-tk", "--truthKNN", "truth set number.");
        AddOptionalOption(m_dataFile, "-df", "--data", "original data file.");
        AddOptionalOption(m_dataFileType, "-dft", "--dataFileType", "original data file type. (TXT, or DEFAULT)");
        AddOptionalOption(m_batch, "-b", "--batchsize", "Batch query size.");
        AddOptionalOption(m_genTruth, "-g", "--gentruth", "Generate truth file.");
        AddOptionalOption(m_debugQuery, "-dq", "--debugquery", "Debug query number.");
        AddOptionalOption(m_enableADC, "-adc", "--adc", "Enable ADC Distance computation");
        AddOptionalOption(m_outputformat, "-of", "--ouputformat", "0: TXT 1: BINARY.");

        // select head

        AddOptionalOption(s_isExecute, "-se", "--h-excute", "bool is execute in head selection section");
        AddOptionalOption(TreeNumber, "-tm", "--tree-number", "number of trees");
        AddOptionalOption(BKTKmeansK, "-bktm", "--bkt-means", "bkt means");
        AddOptionalOption(BKTLeafSize, "-bktls", "--bkt-leaf-size", "bkt leaf size");
        AddOptionalOption(SamplesNumber, "-sn", "--samples-num", "number of samples");
        AddOptionalOption(SelectThreshold, "-st", "--select_th", "select threshold");
        AddOptionalOption(SplitFactor, "-sp", "--split-factor", "split factor");
        AddOptionalOption(SplitThreshold, "-sth", "--split-th", "split threshold");
        AddOptionalOption(Ratio, "-ratio", "--ration", "ration");

        // build head

        AddOptionalOption(b_isExecute, "-be", "--b-excute", "bool is execute in head selection build");
        AddOptionalOption(TPTNumber, "-tptn", "--tpt-number", "number of TPT");
        AddOptionalOption(TPTLeafSize, "-tptlf", "--tpt-leaf-size", "number of tpt leaves");
        AddOptionalOption(b_MaxCheck, "-bmc", "--b-max-check", "max check for build head section");
        AddOptionalOption(MaxCheckForRefineGraph, "-mcfrg", "--mc-refine-graph", "max check for refine graph");
        AddOptionalOption(RefineIterations, "-rt", "--refine-it", "refine iterations");
        AddOptionalOption(NeighborhoodSize, "--in", "--in-deg", "number of in-degree");
    }

    ~Options() {}

    std::string m_queryFile;

    std::string m_indexFolder;

    std::string m_dataFile = "";

    std::string m_truthFile = "";

    std::string m_resultFile = "";

    std::string m_maxCheck = "8192";

    VectorFileType m_dataFileType = VectorFileType::DEFAULT;

    int m_withMeta = 0;

    int m_K = 32;

    int m_truthK = -1;

    int m_batch = 10000;

    int m_genTruth = 0;

    int m_debugQuery = -1;

    bool m_enableADC = false;

    int m_outputformat = 0;

    std::string m_inputFiles;

    std::string m_outputFolder;

    SPTAG::IndexAlgoType m_indexAlgoType;

    std::string m_builderConfigFile;

    std::string m_quantizerFile;

    int m_mode = 0;

    bool m_metaMapping = false;

    int num_points;

    int num_query;

    // SelectHead
    bool s_isExecute = true;
    int TreeNumber = 1;
    int BKTKmeansK = 32;
    int BKTLeafSize = 8;
    int SamplesNumber = 1000;
    int SelectThreshold = 10;
    int SplitFactor = 6;
    int SplitThreshold = 25;
    float Ratio = 0.12;

    // BuildHead
    bool b_isExecute = true;
    int TPTNumber = 32;
    int TPTLeafSize = 2000;
    int b_MaxCheck = 16324;
    int MaxCheckForRefineGraph = 16324;
    int RefineIterations = 3;
    DimensionType NeighborhoodSize = 32;
};

void peak_memory_footprint() {

        unsigned iPid = (unsigned)getpid();

        std::cout<<"PID: "<<iPid<<std::endl;

        std::string status_file = "/proc/" + std::to_string(iPid) + "/status";
        std::ifstream info(status_file);
        if (!info.is_open()) {
            std::cout << "memory information open error!" << std::endl;
        }
        std::string tmp;
        while(getline(info, tmp)) {
            if (tmp.find("Name:") != std::string::npos || tmp.find("VmPeak:") != std::string::npos || tmp.find("VmHWM:") != std::string::npos)
            std::cout << tmp << std::endl;
        }
        info.close();
        
    }

int BuildIndex(std::shared_ptr<Options> options, int argc, char *argv[])
{

    // SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Set QuantizerFile = %s\n", options->m_quantizerFile.c_str());

    auto indexBuilder = VectorIndex::CreateInstance(options->m_indexAlgoType, options->m_inputValueType);
    if (!options->m_quantizerFile.empty())
    {
        indexBuilder->LoadQuantizer(options->m_quantizerFile);
        if (!indexBuilder->m_pQuantizer)
        {
            exit(1);
        }
    }

    Helper::IniReader iniReader;
    if (!options->m_builderConfigFile.empty() && iniReader.LoadIniFile(options->m_builderConfigFile) != ErrorCode::Success)
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot open index configure file!");
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string param(argv[i]);
        size_t idx = param.find("=");
        if (idx == std::string::npos)
            continue;

        std::string paramName = param.substr(0, idx);
        std::string paramVal = param.substr(idx + 1);
        std::string sectionName;
        idx = paramName.find(".");
        if (idx != std::string::npos)
        {
            sectionName = paramName.substr(0, idx);
            paramName = paramName.substr(idx + 1);
        }
        iniReader.SetParameter(sectionName, paramName, paramVal);
        SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Set [%s]%s = %s\n", sectionName.c_str(), paramName.c_str(), paramVal.c_str());
    }

    std::string sections[] = {"Base", "SelectHead", "BuildHead", "BuildSSDIndex", "Index"};
    for (int i = 0; i < 5; i++)
    {
        if (!iniReader.DoesParameterExist(sections[i], "NumberOfThreads"))
        {
            iniReader.SetParameter(sections[i], "NumberOfThreads", std::to_string(options->m_threadNum));
        }
        for (const auto &iter : iniReader.GetParameters(sections[i]))
        {
            indexBuilder->SetParameter(iter.first.c_str(), iter.second.c_str(), sections[i]);
        }
    }

    std::string select_head[] = {"s_isExecute", "TreeNumber", "BKTKmeansK", "BKTLeafSize", "SamplesNumber", "SelectThreshold", "SplitFactor", "SplitThreshold", "Ratio"};
    indexBuilder->SetParameter(select_head[0], std::to_string(options->s_isExecute), "SelectdHead");
    indexBuilder->SetParameter(select_head[1], std::to_string(options->TreeNumber), "SelectdHead");
    indexBuilder->SetParameter(select_head[2], std::to_string(options->BKTKmeansK), "SelectdHead");
    indexBuilder->SetParameter(select_head[3], std::to_string(options->BKTLeafSize), "SelectdHead");
    indexBuilder->SetParameter(select_head[4], std::to_string(options->SamplesNumber), "SelectdHead");
    indexBuilder->SetParameter(select_head[5], std::to_string(options->SelectThreshold), "SelectdHead");
    indexBuilder->SetParameter(select_head[6], std::to_string(options->SplitFactor), "SelectdHead");
    indexBuilder->SetParameter(select_head[7], std::to_string(options->SplitThreshold), "SelectdHead");
    indexBuilder->SetParameter(select_head[8], std::to_string(options->Ratio), "SelectdHead");

    std::string build_head[] = {"b_isExecute", "TPTNumber", "TPTLeafSize", "b_MaxCheck", "MaxCheckForRefineGraph", "RefineIterations", "NeighborhoodSize"};
    indexBuilder->SetParameter(build_head[0], std::to_string(options->b_isExecute), "BuildHead");
    indexBuilder->SetParameter(build_head[1], std::to_string(options->TPTNumber), "BuildHead");
    indexBuilder->SetParameter(build_head[2], std::to_string(options->TPTLeafSize), "BuildHead");
    indexBuilder->SetParameter(build_head[3], std::to_string(options->b_MaxCheck), "BuildHead");
    indexBuilder->SetParameter(build_head[4], std::to_string(options->MaxCheckForRefineGraph), "BuildHead");
    indexBuilder->SetParameter(build_head[5], std::to_string(options->RefineIterations), "BuildHead");
    indexBuilder->SetParameter(build_head[6], std::to_string(options->NeighborhoodSize), "BuildHead");

    ErrorCode code;
    std::shared_ptr<VectorSet> vecset;
    if (options->m_inputFiles != "")
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto vectorReader = Helper::VectorSetReader::CreateInstance(options);
        if (ErrorCode::Success != vectorReader->LoadFile(options->m_inputFiles))
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to read input file.\n");
            exit(1);
        }
        vecset = vectorReader->GetVectorSet2(options->num_points, options->m_dimension);
        indexBuilder->m_mode = 0;
        // std::cout << "vector loaded " << vecset->GetVector(0) << std::endl;
        code = indexBuilder->BuildIndex(vecset, vectorReader->GetMetadataSet(), options->m_metaMapping, options->m_normalized, true);
        auto end = std::chrono::high_resolution_clock::now();
        // std::cout << "treenumber " << indexBuilder->GetParameter("TPTNumber", "BuildHead") << std::endl;
        std::chrono::duration<double> diff = start - end;
        std::cout << "building finished!\n Graph Path => " << options->m_outputFolder << "\n Total time => " << diff.count() << "(sec)" << std::endl;
        peak_memory_footprint();
    }
    else
    {
        indexBuilder->SetQuantizerFileName(options->m_quantizerFile.substr(options->m_quantizerFile.find_last_of("/\\") + 1));
        code = indexBuilder->BuildIndex(options->m_normalized);
    }
    if (code == ErrorCode::Success)
    {
        indexBuilder->SaveIndex(options->m_outputFolder);
    }
    else
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to build index.\n");
        exit(1);
    }
}
/**
template <typename T>
int Process(std::shared_ptr<Options> options, VectorIndex &index)
{
    std::ofstream log("Recall-result.out", std::ios::app);
    if (!log.is_open())
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "ERROR: Cannot open logging file!\n");
        exit(-1);
    }

    auto vectorReader = Helper::VectorSetReader::CreateInstance(options);
    if (ErrorCode::Success != vectorReader->LoadFile(options->m_queryFile))
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to read query file.\n");
        exit(1);
    }
    auto queryVectors = vectorReader->GetVectorSet2(options->num_query, options->m_dimension);
    auto queryMetas = vectorReader->GetMetadataSet();

    std::shared_ptr<Helper::ReaderOptions> dataOptions(new Helper::ReaderOptions(queryVectors->GetValueType(), queryVectors->Dimension(), options->m_dataFileType));
    auto dataReader = Helper::VectorSetReader::CreateInstance(dataOptions);
    std::shared_ptr<VectorSet> dataVectors;
    if (options->m_dataFile != "")
    {
        if (ErrorCode::Success != dataReader->LoadFile(options->m_dataFile))
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to read data file.\n");
            exit(1);
        }
        dataVectors = dataReader->GetVectorSet();
    }

    std::shared_ptr<Helper::DiskIO> ftruth;
    int truthDim = 0;
    if (options->m_truthFile != "")
    {
        if (options->m_genTruth)
        {
            if (dataVectors == nullptr)
            {
                SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot load data vectors to generate groundtruth! Please speicify data vector file by setting -df option.\n");
                exit(1);
            }
            COMMON::TruthSet::GenerateTruth<T>(queryVectors, dataVectors, options->m_truthFile, index.GetDistCalcMethod(), options->m_truthK,
                                               (options->m_truthFile.find("bin") != std::string::npos) ? TruthFileType::DEFAULT : TruthFileType::TXT, index.m_pQuantizer);
        }

        ftruth = SPTAG::f_createIO();
        if (ftruth == nullptr || !ftruth->Initialize(options->m_truthFile.c_str(), std::ios::in | std::ios::binary))
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "ERROR: Cannot open %s for read!\n", options->m_truthFile.c_str());
            exit(1);
        }
        if (options->m_truthFile.find("bin") != std::string::npos)
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Load binary truth...\n");
        }
        else
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Load txt truth...\n");
        }
    }

    std::shared_ptr<Helper::DiskIO> fp;
    if (options->m_resultFile != "")
    {
        fp = SPTAG::f_createIO();
        if (fp == nullptr || !fp->Initialize(options->m_resultFile.c_str(), std::ios::out | std::ios::binary))
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "ERROR: Cannot open %s for write!\n", options->m_resultFile.c_str());
        }

        if (options->m_outputformat == 1)
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Using output format binary...");

            int32_t i32Val = queryVectors->Count();
            if (fp->WriteBinary(sizeof(i32Val), reinterpret_cast<char *>(&i32Val)) != sizeof(i32Val))
            {
                SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Fail to write result file!\n");
                exit(1);
            }
            i32Val = options->m_K;
            if (fp->WriteBinary(sizeof(i32Val), reinterpret_cast<char *>(&i32Val)) != sizeof(i32Val))
            {
                SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Fail to write result file!\n");
                exit(1);
            }
        }
    }

    std::vector<std::string> maxCheck = Helper::StrUtils::SplitString(options->m_maxCheck, "#");
    if (options->m_truthK < 0)
        options->m_truthK = options->m_K;

    std::vector<std::set<SizeType>> truth(options->m_batch);
    int internalResultNum = options->m_K;
    if (index.GetIndexAlgoType() == IndexAlgoType::SPANN)
    {
        int SPANNInternalResultNum;
        if (SPTAG::Helper::Convert::ConvertStringTo<int>(index.GetParameter("SearchInternalResultNum", "BuildSSDIndex").c_str(), SPANNInternalResultNum))
            internalResultNum = max(internalResultNum, SPANNInternalResultNum);
    }
    std::vector<QueryResult> results(options->m_batch, QueryResult(NULL, internalResultNum, options->m_withMeta != 0));
    std::vector<float> latencies(options->m_batch, 0);
    int baseSquare = SPTAG::COMMON::Utils::GetBase<T>() * SPTAG::COMMON::Utils::GetBase<T>();

    SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "[query]\t\t[maxcheck]\t[avg] \t[99%] \t[95%] \t[recall] \t[qps] \t[mem]\n");
    std::vector<float> totalAvg(maxCheck.size(), 0.0), total99(maxCheck.size(), 0.0), total95(maxCheck.size(), 0.0), totalRecall(maxCheck.size(), 0.0), totalLatency(maxCheck.size(), 0.0);
    for (int startQuery = 0; startQuery < queryVectors->Count(); startQuery += options->m_batch)
    {
        int numQuerys = min(options->m_batch, queryVectors->Count() - startQuery);
        for (SizeType i = 0; i < numQuerys; i++)
            results[i].SetTarget(queryVectors->GetVector(startQuery + i));
        if (ftruth != nullptr)
            COMMON::TruthSet::LoadTruth(ftruth, truth, numQuerys, truthDim, options->m_truthK, (options->m_truthFile.find("bin") != std::string::npos) ? TruthFileType::DEFAULT : TruthFileType::TXT);

        for (int mc = 0; mc < maxCheck.size(); mc++)
        {
            index.SetParameter("MaxCheck", maxCheck[mc].c_str());

            for (SizeType i = 0; i < numQuerys; i++)
                results[i].Reset();

            std::atomic_size_t queriesSent(0);
            std::vector<std::thread> threads;

            auto batchstart = std::chrono::high_resolution_clock::now();

            for (std::uint32_t i = 0; i < options->m_threadNum; i++)
            {
                threads.emplace_back([&, i]
                                     {
                    NumaStrategy ns = (index.GetIndexAlgoType() == IndexAlgoType::SPANN)? NumaStrategy::SCATTER: NumaStrategy::LOCAL; // Only for SPANN, we need to avoid IO threads overlap with search threads.
                    Helper::SetThreadAffinity(i, threads[i], ns, OrderStrategy::ASC);

                    size_t qid = 0;
                    while (true)
                    {
                        qid = queriesSent.fetch_add(1);
                        if (qid < numQuerys)
                        {
                            auto t1 = std::chrono::high_resolution_clock::now();
                            // int numberOfDistanceCalculation = 0;
                            index.SearchIndex(results[qid]);
                            // printf("numberOfDistanceCalculation %d \n", numberOfDistanceCalculation);
                            auto t2 = std::chrono::high_resolution_clock::now();
                            latencies[qid] = (float)(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0);
                        }
                        else
                        {
                            return;
                        }
                    } });
            }
            for (auto &thread : threads)
            {
                thread.join();
            }

            auto batchend = std::chrono::high_resolution_clock::now();
            float batchLatency = (float)(std::chrono::duration_cast<std::chrono::microseconds>(batchend - batchstart).count() / 1000000.0);

            float timeMean = 0, timeMin = MaxDist, timeMax = 0, timeStd = 0;
            for (int qid = 0; qid < numQuerys; qid++)
            {
                timeMean += latencies[qid];
                if (latencies[qid] > timeMax)
                    timeMax = latencies[qid];
                if (latencies[qid] < timeMin)
                    timeMin = latencies[qid];
            }
            timeMean /= numQuerys;
            for (int qid = 0; qid < numQuerys; qid++)
                timeStd += ((float)latencies[qid] - timeMean) * ((float)latencies[qid] - timeMean);
            timeStd = std::sqrt(timeStd / numQuerys);
            log << timeMean << " " << timeStd << " " << timeMin << " " << timeMax << " ";

            std::sort(latencies.begin(), latencies.begin() + numQuerys);
            float l99 = latencies[SizeType(numQuerys * 0.99)];
            float l95 = latencies[SizeType(numQuerys * 0.95)];

            float recall = 0;
            if (ftruth != nullptr)
            {
                recall = COMMON::TruthSet::CalculateRecall<T>(&index, results, truth, options->m_K, options->m_truthK, queryVectors, dataVectors, numQuerys, &log, options->m_debugQuery > 0);
            }

#ifndef _MSC_VER
            struct rusage rusage;
            getrusage(RUSAGE_SELF, &rusage);
            unsigned long long peakWSS = rusage.ru_maxrss * 1024 / 1000000000;
#else
            PROCESS_MEMORY_COUNTERS pmc;
            GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
            unsigned long long peakWSS = pmc.PeakWorkingSetSize / 1000000000;
#endif
            SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "%d-%d\t%s\t%.4f\t%.4f\t%.4f\t%.4f\t\t%.4f\t\t%lluGB\n", startQuery, (startQuery + numQuerys), maxCheck[mc].c_str(), timeMean, l99, l95, recall, (numQuerys / batchLatency), peakWSS);
            totalAvg[mc] += timeMean * numQuerys;
            total95[mc] += l95 * numQuerys;
            total99[mc] += l99 * numQuerys;
            totalRecall[mc] += recall * numQuerys;
            totalLatency[mc] += batchLatency;
        }

        if (fp != nullptr)
        {
            if (options->m_outputformat == 0)
            {
                for (SizeType i = 0; i < numQuerys; i++)
                {
                    if (queryMetas != nullptr)
                    {
                        ByteArray qmeta = queryMetas->GetMetadata(startQuery + i);
                        if (fp->WriteBinary(qmeta.Length(), (const char *)qmeta.Data()) != qmeta.Length())
                        {
                            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot write qmeta %d bytes!\n", qmeta.Length());
                            exit(1);
                        }
                    }
                    else
                    {
                        std::string qid = std::to_string(i);
                        if (fp->WriteBinary(qid.length(), qid.c_str()) != qid.length())
                        {
                            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot write qid %d bytes!\n", qid.length());
                            exit(1);
                        }
                    }
                    fp->WriteString(("----------" + std::to_string(options->m_K) + "-NN RESULTS----------- |  :\n").c_str());
                    for (int j = 0; j < options->m_K; j++)
                    {
                        std::string sd = std::to_string(results[i].GetResult(j)->Dist / baseSquare);
                        fp->WriteString(("K N°" + std::to_string(j + 1) + "  => Distance ").c_str());
                        if (fp->WriteBinary(sd.length(), sd.c_str()) != sd.length())
                        {
                            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot write dist %d bytes!\n", sd.length());
                            exit(1);
                        }
                        if (results[i].GetResult(j)->VID < 0)
                        {
                            fp->WriteString("NULL|");
                            continue;
                        }

                        fp->WriteString(" | Node ID: ");
                        if (!options->m_withMeta)
                        {
                            std::string vid = std::to_string(results[i].GetResult(j)->VID);
                            if (fp->WriteBinary(vid.length(), vid.c_str()) != vid.length())
                            {
                                SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot write vid %d bytes!\n", sd.length());
                                exit(1);
                            }
                        }
                        else
                        {
                            ByteArray vm = index.GetMetadata(results[i].GetResult(j)->VID);
                            if (fp->WriteBinary(vm.Length(), (const char *)vm.Data()) != vm.Length())
                            {
                                SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot write vmeta %d bytes!\n", vm.Length());
                                exit(1);
                            }
                        }
                        fp->WriteString((" | Time :" + std::to_string(latencies[i]) + "\n").c_str());
                    }
                    fp->WriteString("\n");
                }
            }
            else
            {
                for (SizeType i = 0; i < numQuerys; ++i)
                {
                    for (int j = 0; j < options->m_K; ++j)
                    {
                        SizeType i32Val = results[i].GetResult(j)->VID;
                        if (fp->WriteBinary(sizeof(i32Val), reinterpret_cast<char *>(&i32Val)) != sizeof(i32Val))
                        {
                            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Fail to write result file!\n");
                            exit(1);
                        }

                        float fVal = results[i].GetResult(j)->Dist;
                        if (fp->WriteBinary(sizeof(fVal), reinterpret_cast<char *>(&fVal)) != sizeof(fVal))
                        {
                            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Fail to write result file!\n");
                            exit(1);
                        }
                    }
                }
            }
        }
    }
    for (int mc = 0; mc < maxCheck.size(); mc++)
        SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "%d-%d\t%s\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", 0, queryVectors->Count(), maxCheck[mc].c_str(), (totalAvg[mc] / queryVectors->Count()), (total99[mc] / queryVectors->Count()), (total95[mc] / queryVectors->Count()), (totalRecall[mc] / queryVectors->Count()), (queryVectors->Count() / totalLatency[mc]));

    SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Output results finish!\n");

    if (fp != nullptr)
        fp->ShutDown();
    log.close();
    return 0;
}
**/



template <typename T>
int Process(std::shared_ptr<Options> options, VectorIndex &index) {
    index.m_mode = 1;
    auto vectorReader = Helper::VectorSetReader::CreateInstance(options);
    if (ErrorCode::Success != vectorReader->LoadFile(options->m_queryFile))
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to read query file.\n");
        exit(1);
    }
    auto queryVectors = vectorReader->GetVectorSet2(options->num_query, options->m_dimension);
    auto queryMetas = vectorReader->GetMetadataSet();

    std::shared_ptr<Helper::ReaderOptions> dataOptions(new Helper::ReaderOptions(queryVectors->GetValueType(), queryVectors->Dimension(), options->m_dataFileType));
    auto dataReader = Helper::VectorSetReader::CreateInstance(dataOptions);

    std::shared_ptr<VectorSet> dataVectors;
    if (options->m_dataFile != "")
    {
        if (ErrorCode::Success != dataReader->LoadFile(options->m_dataFile))
        {
            SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Failed to read data file.\n");
            exit(1);
        }
        dataVectors = dataReader->GetVectorSet();
    }
    std::vector<std::string> maxCheck = Helper::StrUtils::SplitString(options->m_maxCheck, "#");

    int internalResultNum = options->m_K;

    std::vector<QueryResult> results(queryVectors->Count() + 5, QueryResult(NULL, internalResultNum, options->m_withMeta != 0));
    for(int i = 0; i < queryVectors->Count(); i++){
        results[i].SetTarget(queryVectors->GetVector(i));
            index.SetParameter("MaxCheck", maxCheck[maxCheck.size()-1].c_str());
            results[i].Reset();
            printf(" %d ---------- %d-NN RESULTS----------- |  :\n", i, options->m_K);
            index.SearchIndex(results[i]);        
    }
    peak_memory_footprint();
}

int SearchIndex(std::shared_ptr<Options> options, int argc, char *argv[])
{

    std::shared_ptr<SPTAG::VectorIndex> vecIndex;
    auto ret = SPTAG::VectorIndex::LoadIndex(options->m_indexFolder, vecIndex);
    if (SPTAG::ErrorCode::Success != ret || nullptr == vecIndex)
    {
        SPTAGLIB_LOG(Helper::LogLevel::LL_Error, "Cannot open index configure file!");
        return -1;
    }
    vecIndex->SetQuantizerADC(options->m_enableADC);

    Helper::IniReader iniReader;
    for (int i = 1; i < argc; i++)
    {
        std::string param(argv[i]);
        size_t idx = param.find("=");
        if (idx == std::string::npos)
            continue;

        std::string paramName = param.substr(0, idx);
        std::string paramVal = param.substr(idx + 1);
        std::string sectionName;
        idx = paramName.find(".");
        if (idx != std::string::npos)
        {
            sectionName = paramName.substr(0, idx);
            paramName = paramName.substr(idx + 1);
        }
        iniReader.SetParameter(sectionName, paramName, paramVal);
        SPTAGLIB_LOG(Helper::LogLevel::LL_Info, "Set [%s]%s = %s\n", sectionName.c_str(), paramName.c_str(), paramVal.c_str());
    }

    std::string sections[] = {"Base", "SelectHead", "BuildHead", "BuildSSDIndex", "Index"};
    for (int i = 0; i < 5; i++)
    {
        if (!iniReader.DoesParameterExist(sections[i], "NumberOfThreads"))
        {
            iniReader.SetParameter(sections[i], "NumberOfThreads", std::to_string(options->m_threadNum));
        }
        for (const auto &iter : iniReader.GetParameters(sections[i]))
        {
            vecIndex->SetParameter(iter.first.c_str(), iter.second.c_str(), sections[i]);
        }
    }

    vecIndex->SetParameter("m_maxCheck", options->m_maxCheck, "Base");

    vecIndex->UpdateIndex();
    auto start = std::chrono::high_resolution_clock::now();

    switch (options->m_inputValueType)
    {
#define DefineVectorValueType(Name, Type)          \
    case VectorValueType::Name:                    \
        Process<Type>(options, *(vecIndex.get())); \
        break;

#include "inc/Core/DefinitionList.h"
#undef DefineVectorValueType

    default:
        break;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = start - end;
    std::cout << "Querying finished!  "
              << "\n Total time => " << diff.count() << "(sec)" << std::endl;
}

int main(int argc, char *argv[])
{
    std::shared_ptr<Options> options(new Options);

    if (!options->Parse(argc - 1, argv + 1))
    {
        exit(1);
    }

    if (options->m_mode == 0)
    {
        printf("inside build mode \n");
        BuildIndex(options, argc, argv);
        peak_memory_footprint();

    }
    else if (options->m_mode == 1)
    {
        SearchIndex(options, argc, argv);
    }
    peak_memory_footprint();
    return 0;
}
