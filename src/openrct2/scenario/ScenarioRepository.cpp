/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "ScenarioRepository.h"

#include "../Context.h"
#include "../Diagnostic.h"
#include "../Game.h"
#include "../ParkImporter.h"
#include "../PlatformEnvironment.h"
#include "../config/Config.h"
#include "../core/Console.hpp"
#include "../core/File.h"
#include "../core/FileIndex.hpp"
#include "../core/FileStream.h"
#include "../core/MemoryStream.h"
#include "../core/Numerics.hpp"
#include "../core/Path.hpp"
#include "../core/String.hpp"
#include "../localisation/LocalisationService.h"
#include "../platform/Platform.h"
#include "../rct12/CSStringConverter.h"
#include "../rct12/RCT12.h"
#include "../rct12/SawyerChunkReader.h"
#include "../rct2/RCT2.h"
#include "Scenario.h"
#include "ScenarioSources.h"

#include <memory>
#include <string>
#include <vector>

using namespace OpenRCT2;

static int32_t ScenarioCategoryCompare(ScenarioCategory categoryA, ScenarioCategory categoryB)
{
    if (categoryA == categoryB)
        return 0;
    if (categoryA == ScenarioCategory::dlc)
        return -1;
    if (categoryB == ScenarioCategory::dlc)
        return 1;
    if (categoryA == ScenarioCategory::buildYourOwn)
        return -1;
    if (categoryB == ScenarioCategory::buildYourOwn)
        return 1;
    if (categoryA < categoryB)
        return -1;
    return 1;
}

static int32_t ScenarioIndexEntryCompareByCategory(const ScenarioIndexEntry& entryA, const ScenarioIndexEntry& entryB)
{
    // Order by category
    if (entryA.Category != entryB.Category)
    {
        return ScenarioCategoryCompare(entryA.Category, entryB.Category);
    }

    // Then by source game / name
    switch (entryA.Category)
    {
        default:
            if (entryA.SourceGame != entryB.SourceGame)
            {
                return static_cast<int32_t>(entryA.SourceGame) - static_cast<int32_t>(entryB.SourceGame);
            }
            return strcmp(entryA.Name.c_str(), entryB.Name.c_str());
        case ScenarioCategory::real:
        case ScenarioCategory::other:
            return strcmp(entryA.Name.c_str(), entryB.Name.c_str());
    }
}

static int32_t ScenarioIndexEntryCompareByIndex(const ScenarioIndexEntry& entryA, const ScenarioIndexEntry& entryB)
{
    // Order by source game
    if (entryA.SourceGame != entryB.SourceGame)
    {
        return static_cast<int32_t>(entryA.SourceGame) - static_cast<int32_t>(entryB.SourceGame);
    }

    // Then by index / category / name
    ScenarioSource sourceGame = ScenarioSource{ entryA.SourceGame };
    switch (sourceGame)
    {
        default:
            if (entryA.SourceIndex == -1 && entryB.SourceIndex == -1)
            {
                if (entryA.Category == entryB.Category)
                {
                    return ScenarioIndexEntryCompareByCategory(entryA, entryB);
                }

                return ScenarioCategoryCompare(entryA.Category, entryB.Category);
            }
            if (entryA.SourceIndex == -1)
            {
                return 1;
            }
            if (entryB.SourceIndex == -1)
            {
                return -1;
            }
            return entryA.SourceIndex - entryB.SourceIndex;

        case ScenarioSource::Real:
            return ScenarioIndexEntryCompareByCategory(entryA, entryB);
    }
}

static void ScenarioHighscoreFree(ScenarioHighscoreEntry* highscore)
{
    delete highscore;
}

class ScenarioFileIndex final : public FileIndex<ScenarioIndexEntry>
{
private:
    static constexpr uint32_t kMagicNumber = 0x58444953; // SIDX
    static constexpr uint16_t kVersion = 9;
    static constexpr auto kPattern = "*.sc4;*.sc6;*.sea;*.park";

public:
    explicit ScenarioFileIndex(const IPlatformEnvironment& env)
        : FileIndex(
              "scenario index", kMagicNumber, kVersion, env.GetFilePath(PathId::cacheScenarios), std::string(kPattern),
              std::vector<std::string>({
                  env.GetDirectoryPath(DirBase::rct1, DirId::scenarios),
                  env.GetDirectoryPath(DirBase::rct2, DirId::scenarios),
                  env.GetDirectoryPath(DirBase::user, DirId::scenarios),
              }))
    {
    }

protected:
    std::optional<ScenarioIndexEntry> Create(int32_t, const std::string& path) const override
    {
        ScenarioIndexEntry entry;
        auto timestamp = File::GetLastModified(path);
        if (GetScenarioInfo(path, timestamp, &entry))
        {
            return entry;
        }

        return std::nullopt;
    }

    void Serialise(DataSerialiser& ds, const ScenarioIndexEntry& item) const override
    {
        ds << item.Path;
        if (ds.IsLoading())
        {
            // Field used to be fixed size length, remove the 0 padding.
            const auto pos = item.Path.find('\0');
            if (pos != std::string::npos)
            {
                const_cast<ScenarioIndexEntry&>(item).Path = item.Path.substr(0, pos);
            }
        }
        ds << item.Timestamp;
        ds << item.Category;
        ds << item.SourceGame;
        ds << item.SourceIndex;
        ds << item.ScenarioId;
        ds << item.ObjectiveType;
        ds << item.ObjectiveArg1;
        ds << item.ObjectiveArg2;
        ds << item.ObjectiveArg3;

        ds << item.InternalName;
        ds << item.Name;
        ds << item.Details;
    }

private:
    static std::unique_ptr<IStream> GetStreamFromRCT2Scenario(const std::string& path)
    {
        if (String::iequals(Path::GetExtension(path), ".sea"))
        {
            auto data = DecryptSea(fs::u8path(path));
            auto ms = std::make_unique<MemoryStream>();
            // Need to copy the data into MemoryStream as the overload will borrow instead of copy.
            ms->Write(data.data(), data.size());
            ms->SetPosition(0);
            return ms;
        }

        auto fs = std::make_unique<FileStream>(path, FileMode::open);
        return fs;
    }

    /**
     * Reads basic information from a scenario file.
     */
    static bool GetScenarioInfo(const std::string& path, uint64_t timestamp, ScenarioIndexEntry* entry)
    {
        LOG_VERBOSE("GetScenarioInfo(%s, %d, ...)", path.c_str(), timestamp);
        try
        {
            auto& objRepository = OpenRCT2::GetContext()->GetObjectRepository();
            std::unique_ptr<IParkImporter> importer;
            std::string extension = Path::GetExtension(path);

            if (String::iequals(extension, ".park"))
            {
                importer = ParkImporter::CreateParkFile(objRepository);
                importer->LoadScenario(path, true);
            }
            else if (String::iequals(extension, ".sc4"))
            {
                importer = ParkImporter::CreateS4();
                importer->LoadScenario(path, true);
            }
            else
            {
                importer = ParkImporter::CreateS6(objRepository);
                auto stream = GetStreamFromRCT2Scenario(path);
                importer->LoadFromStream(stream.get(), true);
            }

            if (importer)
            {
                if (importer->PopulateIndexEntry(entry))
                {
                    entry->Path = path;
                    entry->Timestamp = timestamp;
                    return true;
                }
            }

            LOG_VERBOSE("%s is not a scenario", path.c_str());
            return false;
        }
        catch (const std::exception&)
        {
            Console::Error::WriteLine("Unable to read scenario: '%s'", path.c_str());
        }
        return false;
    }
};

class ScenarioRepository final : public IScenarioRepository
{
private:
    static constexpr uint32_t HighscoreFileVersion = 2;

    IPlatformEnvironment& _env;
    ScenarioFileIndex const _fileIndex;
    std::vector<ScenarioIndexEntry> _scenarios;
    std::vector<ScenarioHighscoreEntry*> _highscores;

public:
    explicit ScenarioRepository(IPlatformEnvironment& env)
        : _env(env)
        , _fileIndex(env)
    {
    }

    virtual ~ScenarioRepository()
    {
        ClearHighscores();
    }

    void Scan(int32_t language) override
    {
        ImportMegaPark();

        // Reload scenarios from index
        _scenarios.clear();
        auto scenarios = _fileIndex.LoadOrBuild(language);
        for (const auto& scenario : scenarios)
        {
            AddScenario(scenario);
        }

        // Sort the scenarios and load the highscores
        Sort();
        LoadScores();
        LoadLegacyScores();
        AttachHighscores();
    }

    size_t GetCount() const override
    {
        return _scenarios.size();
    }

    const ScenarioIndexEntry* GetByIndex(size_t index) const override
    {
        const ScenarioIndexEntry* result = nullptr;
        if (index < _scenarios.size())
        {
            result = &_scenarios[index];
        }
        return result;
    }

    const ScenarioIndexEntry* GetByFilename(u8string_view filename) const override
    {
        for (const auto& scenario : _scenarios)
        {
            const auto scenarioFilename = Path::GetFileName(scenario.Path);

            // Note: this is always case insensitive search for cross platform consistency
            if (String::iequals(filename, scenarioFilename))
            {
                return &scenario;
            }
        }
        return nullptr;
    }

    const ScenarioIndexEntry* GetByInternalName(u8string_view name) const override
    {
        for (size_t i = 0; i < _scenarios.size(); i++)
        {
            const ScenarioIndexEntry* scenario = &_scenarios[i];

            if (scenario->SourceGame == ScenarioSource::Other && scenario->ScenarioId == SC_UNIDENTIFIED)
                continue;

            // Note: this is always case insensitive search for cross platform consistency
            if (String::iequals(name, scenario->InternalName))
            {
                return &_scenarios[i];
            }
        }
        return nullptr;
    }

    const ScenarioIndexEntry* GetByPath(const utf8* path) const override
    {
        for (const auto& scenario : _scenarios)
        {
            if (Path::Equals(path, scenario.Path))
            {
                return &scenario;
            }
        }
        return nullptr;
    }

    bool TryRecordHighscore(int32_t language, const utf8* scenarioFileName, money64 companyValue, const utf8* name) override
    {
        // Scan the scenarios so we have a fresh list to query. This is to prevent the issue of scenario completions
        // not getting recorded, see #4951.
        Scan(language);

        ScenarioIndexEntry* scenario = GetByFilename(scenarioFileName);

        if (scenario == nullptr)
        {
            const std::string scenarioBaseName = Path::GetFileNameWithoutExtension(scenarioFileName);
            const std::string scenarioExtension = Path::GetExtension(scenarioFileName);

            // Check if this is an RCTC scenario that corresponds to a known RCT1/2 scenario or vice versa, see #12626
            if (String::iequals(scenarioExtension, ".sea"))
            {
                // Get scenario using RCT2 style name of RCTC scenario
                scenario = GetByFilename((scenarioBaseName + ".sc6").c_str());
            }
            else if (String::iequals(scenarioExtension, ".sc6"))
            {
                // Get scenario using RCTC style name of RCT2 scenario
                scenario = GetByFilename((scenarioBaseName + ".sea").c_str());
            }
            // GameState_t::scenarioFileName .Park scenarios is the full file path instead of just <scenarioName.park>, so need
            // to convert
            else if (String::iequals(scenarioExtension, ".park"))
            {
                scenario = GetByFilename((scenarioBaseName + ".park").c_str());
            }
        }

        if (scenario != nullptr)
        {
            // Check if record company value has been broken or the highscore is the same but no name is registered
            ScenarioHighscoreEntry* highscore = scenario->Highscore;
            if (highscore == nullptr || companyValue > highscore->company_value
                || (highscore->name.empty() && companyValue == highscore->company_value))
            {
                if (highscore == nullptr)
                {
                    highscore = InsertHighscore();
                    highscore->timestamp = Platform::GetDatetimeNowUTC();
                    scenario->Highscore = highscore;
                }
                else
                {
                    if (!highscore->name.empty())
                    {
                        highscore->timestamp = Platform::GetDatetimeNowUTC();
                    }
                }
                highscore->fileName = Path::GetFileName(scenario->Path);
                highscore->name = name != nullptr ? name : "";
                highscore->company_value = companyValue;
                SaveHighscores();
                return true;
            }
        }
        return false;
    }

private:
    ScenarioIndexEntry* GetByFilename(u8string_view filename)
    {
        for (auto& scenario : _scenarios)
        {
            const auto scenarioFilename = Path::GetFileName(scenario.Path);

            // Note: this is always case insensitive search for cross platform consistency
            if (String::iequals(filename, scenarioFilename))
            {
                return &scenario;
            }
        }
        return nullptr;
    }

    ScenarioIndexEntry* GetByPath(const utf8* path)
    {
        const ScenarioRepository* repo = this;
        return const_cast<ScenarioIndexEntry*>(repo->GetByPath(path));
    }

    /**
     * Mega Park from RollerCoaster Tycoon 1 is stored in an encrypted hidden file: mp.dat.
     * Decrypt the file and save it as sc21.sc4 in the user's scenario directory.
     */
    void ImportMegaPark()
    {
        auto mpdatPath = _env.FindFile(DirBase::rct1, DirId::data, "mp.dat");
        if (File::Exists(mpdatPath))
        {
            auto scenarioDirectory = _env.GetDirectoryPath(DirBase::user, DirId::scenarios);
            auto expectedSc21Path = Path::Combine(scenarioDirectory, "sc21.sc4");
            auto sc21Path = Path::ResolveCasing(expectedSc21Path);
            if (!File::Exists(sc21Path))
            {
                ConvertMegaPark(mpdatPath, expectedSc21Path);
            }
        }
    }

    /**
     * Converts Mega Park to normalised file location (mp.dat to sc21.sc4)
     * @param srcPath Full path to mp.dat
     * @param dstPath Full path to sc21.dat
     */
    void ConvertMegaPark(const std::string& srcPath, const std::string& dstPath)
    {
        Path::CreateDirectory(Path::GetDirectory(dstPath));

        auto mpdat = File::ReadAllBytes(srcPath);

        // Rotate each byte of mp.dat left by 4 bits to convert
        for (size_t i = 0; i < mpdat.size(); i++)
        {
            mpdat[i] = Numerics::rol8(mpdat[i], 4);
        }

        File::WriteAllBytes(dstPath, mpdat.data(), mpdat.size());
    }

    void AddScenario(const ScenarioIndexEntry& entry)
    {
        auto filename = Path::GetFileName(entry.Path);

        if (!String::equals(filename, ""))
        {
            auto existingEntry = GetByFilename(filename.c_str());
            if (existingEntry != nullptr)
            {
                std::string conflictPath;
                if (existingEntry->Timestamp > entry.Timestamp)
                {
                    // Existing entry is more recent
                    conflictPath = existingEntry->Path;

                    // Overwrite existing entry with this one
                    *existingEntry = entry;
                }
                else
                {
                    // This entry is more recent
                    conflictPath = entry.Path;
                }
                Console::WriteLine("Scenario conflict: '%s' ignored because it is newer.", conflictPath.c_str());
            }
            else
            {
                _scenarios.push_back(entry);
            }
        }
        else
        {
            LOG_ERROR("Tried to add scenario with an empty filename!");
        }
    }

    void Sort()
    {
        std::sort(_scenarios.begin(), _scenarios.end(), [](const ScenarioIndexEntry& a, const ScenarioIndexEntry& b) -> bool {
            return ScenarioIndexEntryCompareByIndex(a, b) < 0;
        });
    }

    void LoadScores()
    {
        std::string path = _env.GetFilePath(PathId::scores);
        if (!File::Exists(path))
        {
            return;
        }

        try
        {
            auto fs = FileStream(path, FileMode::open);
            uint32_t fileVersion = fs.ReadValue<uint32_t>();
            if (fileVersion != 1 && fileVersion != 2)
            {
                Console::Error::WriteLine("Invalid or incompatible highscores file.");
                return;
            }

            ClearHighscores();

            uint32_t numHighscores = fs.ReadValue<uint32_t>();
            for (uint32_t i = 0; i < numHighscores; i++)
            {
                ScenarioHighscoreEntry* highscore = InsertHighscore();
                highscore->fileName = fs.ReadStdString();
                highscore->name = fs.ReadStdString();
                highscore->company_value = fileVersion == 1 ? fs.ReadValue<money32>() : fs.ReadValue<money64>();
                highscore->timestamp = fs.ReadValue<datetime64>();
            }
        }
        catch (const std::exception&)
        {
            Console::Error::WriteLine("Error reading highscores.");
        }
    }

    /**
     * Loads the original scores.dat file and replaces any highscores that
     * are better for matching scenarios.
     */
    void LoadLegacyScores()
    {
        std::string rct2Path = _env.GetFilePath(PathId::scoresRCT2);
        std::string legacyPath = _env.GetFilePath(PathId::scoresLegacy);
        LoadLegacyScores(legacyPath);
        LoadLegacyScores(rct2Path);
    }

    void LoadLegacyScores(const std::string& path)
    {
        if (!File::Exists(path))
        {
            return;
        }

        bool highscoresDirty = false;
        try
        {
            auto fs = FileStream(path, FileMode::open);
            if (fs.GetLength() <= 4)
            {
                // Initial value of scores for RCT2, just ignore
                return;
            }

            // Load header
            auto header = fs.ReadValue<RCT2::ScoresHeader>();
            for (uint32_t i = 0; i < header.ScenarioCount; i++)
            {
                // Read legacy entry
                auto scBasic = fs.ReadValue<RCT2::ScoresEntry>();

                // Ignore non-completed scenarios
                if (scBasic.Flags & SCENARIO_FLAGS_COMPLETED)
                {
                    bool notFound = true;
                    for (auto& highscore : _highscores)
                    {
                        if (String::iequals(scBasic.Path, highscore->fileName))
                        {
                            notFound = false;

                            // Check if legacy highscore is better
                            if (scBasic.CompanyValue > highscore->company_value)
                            {
                                std::string name = RCT2StringToUTF8(scBasic.CompletedBy, RCT2LanguageId::EnglishUK);
                                highscore->name = name;
                                highscore->company_value = scBasic.CompanyValue;
                                highscore->timestamp = kDatetime64Min;
                                break;
                            }
                        }
                    }
                    if (notFound)
                    {
                        ScenarioHighscoreEntry* highscore = InsertHighscore();
                        highscore->fileName = scBasic.Path;
                        std::string name = RCT2StringToUTF8(scBasic.CompletedBy, RCT2LanguageId::EnglishUK);
                        highscore->name = name;
                        highscore->company_value = scBasic.CompanyValue;
                        highscore->timestamp = kDatetime64Min;
                    }
                }
            }
        }
        catch (const std::exception&)
        {
            Console::Error::WriteLine("Error reading legacy scenario scores file: '%s'", path.c_str());
        }

        if (highscoresDirty)
        {
            SaveHighscores();
        }
    }

    void ClearHighscores()
    {
        for (auto highscore : _highscores)
        {
            ScenarioHighscoreFree(highscore);
        }
        _highscores.clear();
    }

    ScenarioHighscoreEntry* InsertHighscore()
    {
        auto highscore = new ScenarioHighscoreEntry();
        _highscores.push_back(highscore);
        return highscore;
    }

    void AttachHighscores()
    {
        for (auto& highscore : _highscores)
        {
            ScenarioIndexEntry* scenario = GetByFilename(highscore->fileName);
            if (scenario != nullptr)
            {
                scenario->Highscore = highscore;
            }
        }
    }

    void SaveHighscores()
    {
        std::string path = _env.GetFilePath(PathId::scores);
        try
        {
            auto fs = FileStream(path, FileMode::write);
            fs.WriteValue<uint32_t>(HighscoreFileVersion);
            fs.WriteValue<uint32_t>(static_cast<uint32_t>(_highscores.size()));
            for (size_t i = 0; i < _highscores.size(); i++)
            {
                const ScenarioHighscoreEntry* highscore = _highscores[i];
                fs.WriteString(highscore->fileName);
                fs.WriteString(highscore->name);
                fs.WriteValue(highscore->company_value);
                fs.WriteValue(highscore->timestamp);
            }
        }
        catch (const std::exception&)
        {
            Console::Error::WriteLine("Unable to save highscores to '%s'", path.c_str());
        }
    }
};

std::unique_ptr<IScenarioRepository> CreateScenarioRepository(IPlatformEnvironment& env)
{
    return std::make_unique<ScenarioRepository>(env);
}

IScenarioRepository* GetScenarioRepository()
{
    return GetContext()->GetScenarioRepository();
}

void ScenarioRepositoryScan()
{
    IScenarioRepository* repo = GetScenarioRepository();
    repo->Scan(LocalisationService_GetCurrentLanguage());
}

size_t ScenarioRepositoryGetCount()
{
    IScenarioRepository* repo = GetScenarioRepository();
    return repo->GetCount();
}

const ScenarioIndexEntry* ScenarioRepositoryGetByIndex(size_t index)
{
    IScenarioRepository* repo = GetScenarioRepository();
    return repo->GetByIndex(index);
}

bool ScenarioRepositoryTryRecordHighscore(const utf8* scenarioFileName, money64 companyValue, const utf8* name)
{
    IScenarioRepository* repo = GetScenarioRepository();
    return repo->TryRecordHighscore(LocalisationService_GetCurrentLanguage(), scenarioFileName, companyValue, name);
}
