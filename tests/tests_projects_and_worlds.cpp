//
// Created by robin on 03.08.2020.
//

#include "../external_libs/catch.hpp"
#include "../TilesonConfig.h"

#ifdef TILESON_UNIT_TEST_USE_SINGLE_HEADER
    #include "../tileson.hpp"
#else
    #include "../include/tileson.h"
#endif

#include "TestTools.hpp"

TEST_CASE( "Parse world - Expect 4 maps and parsed data", "[project][world]" )
{
    fs::path pathToUse = GetPathWithBase(fs::path("test-maps/project/world/test.world"));

    tson::World world {pathToUse};
    REQUIRE(world.getMapData().size() == 4);
    REQUIRE(world.get("not_exists") == nullptr);
    REQUIRE(world.get("w1.json") != nullptr);
    REQUIRE(world.get("w1.json")->size == tson::Vector2i (256, 128));
    REQUIRE(world.get("w1.json")->position == tson::Vector2i (-256, -128));
    REQUIRE(!world.onlyShowAdjacentMaps());
    REQUIRE(world.getType() == "world");
}

TEST_CASE( "Parse project - expect right number of files and folders", "[project][world]" )
{

    fs::path pathToUse = GetPathWithBase(fs::path("test-maps/project/test.tiled-project"));

    tson::Project project {pathToUse};
    auto files = project.getFolders().at(0).getSubFolders().at(0).getFiles();
    std::string str;
    std::for_each(files.begin(), files.end(), [&](const auto &item) { str.append(item.generic_string()); });

    REQUIRE(project.getData().folders.size() == 2);
    REQUIRE(project.getFolders().size() == 2);
    REQUIRE(project.getFolders().at(0).getPath().filename() == "maps");
    REQUIRE(!project.getFolders().at(0).hasWorldFile());
    REQUIRE(project.getFolders().at(0).getFiles().size() == 3);
    REQUIRE(project.getFolders().at(0).getSubFolders().size() == 1);
    REQUIRE(project.getFolders().at(0).getSubFolders().at(0).getFiles().size() == 1);
    REQUIRE(project.getFolders().at(1).getPath().filename() == "world");
    REQUIRE(project.getFolders().at(1).hasWorldFile());
    REQUIRE(project.getFolders().at(1).getFiles().size() == 4);
    REQUIRE(project.getFolders().at(1).getSubFolders().empty());
}

void tiledProjectEnumAndClassBaseTest(tson::Map *m)
{
    REQUIRE(m->getStatus() == tson::ParseStatus::OK);
    auto c = m->get<tson::TiledClass>("classdata");

    //Asserts
    int age = c.get<int>("Age");
    fs::path extraFile = c.get<fs::path>("ExtraFile");
    uint32_t myObject = c.get<uint32_t>("MyObject");
    std::string name = c.get<std::string>("Name");
    bool canDestroy = c.get<bool>("CanDestroy");
    float money = c.get<float>("MoneyInBag");
    tson::Colori color = c.get<tson::Colori>("ShoeColor");
    tson::EnumValue classEnumNumber = c.get<tson::EnumValue>("NumFlag");
    tson::EnumValue classEnumString = c.get<tson::EnumValue>("StrFlag");

    //Check class stuff
    REQUIRE(age == 51);
    REQUIRE(extraFile.filename() == "ultimate_test_v1.5.json");
    REQUIRE(myObject == 1);
    REQUIRE(name == "Karen Modifini");
    REQUIRE(canDestroy);
    REQUIRE(tson::Tools::Equal(money, 16.9344f));
    REQUIRE(color == "#ff069504");
    REQUIRE(classEnumNumber.getValue() == 10);
    REQUIRE(classEnumString.getValue() == 6);

    //Check standalone enum stuff
    tson::EnumValue enumNumber = m->get<tson::EnumValue>("enumnumber");
    tson::EnumValue enumNumberFlags = m->get<tson::EnumValue>("enumnumberflags");
    tson::EnumValue enumStr = m->get<tson::EnumValue>("enumstr");
    tson::EnumValue enumStrFlags = m->get<tson::EnumValue>("enumstrflags");

    REQUIRE(enumNumber.getValue() == 3);
    REQUIRE(enumStr.getValue() == 0);
    REQUIRE(enumStr.getValueName() == "CreatePlayer");
    REQUIRE(enumNumber.getValueName() == "GetNumber");
    REQUIRE(enumNumber.containsValueName("GetNumber"));
    REQUIRE(enumNumberFlags.getValue() == 7);
    REQUIRE(enumNumberFlags.getValueNames().size() == 3);
    std::vector<std::string> expectedNumberFlagNames {"HasCalculatorFlag", "HasBombFlag", "HasHumorFlag"};
    for(const std::string &s : expectedNumberFlagNames)
    {
        REQUIRE(enumNumberFlags.containsValueName(s));
    }
    REQUIRE(enumStrFlags.getValue() == 7);
    REQUIRE(enumStrFlags.getValueNames().size() == 3);
}

TEST_CASE( "Parse project with class and enum info in maps - expect right values", "[project][map]" )
{
    fs::path pathToUse = GetPathWithBase(fs::path("test-maps/project/test.tiled-project"));

    tson::Project project {pathToUse};
    auto files = project.getFolders().at(0).getSubFolders().at(0).getFiles();
    std::string str;
    std::for_each(files.begin(), files.end(), [&](const auto &item) { str.append(item.generic_string()); });

    REQUIRE(project.getData().folders.size() == 2);
    REQUIRE(project.getFolders().size() == 2);
    REQUIRE(project.getFolders().at(0).getPath().filename() == "maps");
    REQUIRE(!project.getFolders().at(0).hasWorldFile());
    REQUIRE(project.getFolders().at(0).getFiles().size() == 3);
    REQUIRE(project.getFolders().at(0).getSubFolders().size() == 1);
    REQUIRE(project.getFolders().at(0).getSubFolders().at(0).getFiles().size() == 1);
    REQUIRE(project.getFolders().at(1).getPath().filename() == "world");
    REQUIRE(project.getFolders().at(1).hasWorldFile());
    REQUIRE(project.getFolders().at(1).getFiles().size() == 4);
    REQUIRE(project.getFolders().at(1).getSubFolders().empty());

    auto folderFiles = project.getFolders().at(0).getFiles();
    for(fs::path &f : folderFiles)
    {
        fs::path path = project.getFolders().at(0).getPath() / f.filename();
        std::string filename = f.filename().generic_string();
        if(filename == "map1.json")
        {
            tson::Tileson t {&project};
            REQUIRE(fs::exists(path));
            std::unique_ptr<tson::Map> m = t.parse(path);
            tiledProjectEnumAndClassBaseTest(m.get());
        }
    }
}


TEST_CASE( "Parse Tiled v1.9 project with class and enum info in maps - expect right values", "[project][map]" )
{
    fs::path pathToUse = GetPathWithBase(fs::path("test-maps/project-v1.9/test.tiled-project"));

    tson::Project project {pathToUse};
    auto files = project.getFolders().at(0).getSubFolders().at(0).getFiles();
    std::string str;
    std::for_each(files.begin(), files.end(), [&](const auto &item) { str.append(item.generic_string()); });

    REQUIRE(project.getData().folders.size() == 2);
    REQUIRE(project.getFolders().size() == 2);
    REQUIRE(project.getFolders().at(0).getPath().filename() == "maps");
    REQUIRE(!project.getFolders().at(0).hasWorldFile());
    REQUIRE(project.getFolders().at(0).getFiles().size() == 3);
    REQUIRE(project.getFolders().at(0).getSubFolders().size() == 1);
    REQUIRE(project.getFolders().at(0).getSubFolders().at(0).getFiles().size() == 1);
    REQUIRE(project.getFolders().at(1).getPath().filename() == "world");
    REQUIRE(project.getFolders().at(1).hasWorldFile());
    REQUIRE(project.getFolders().at(1).getFiles().size() == 4);
    REQUIRE(project.getFolders().at(1).getSubFolders().empty());

    auto folderFiles = project.getFolders().at(0).getFiles();
    for(fs::path &f : folderFiles)
    {
        fs::path path = project.getFolders().at(0).getPath() / f.filename();
        std::string filename = f.filename().generic_string();
        if(filename == "map1.json")
        {
            tson::Tileson t {&project};
            REQUIRE(fs::exists(path));
            std::unique_ptr<tson::Map> m = t.parse(path);
            tiledProjectEnumAndClassBaseTest(m.get());
            //Should be null, as this is not initialized from a project, but should not crash

            tson::Layer *objectLayer = m->getLayer("Da Object Layer");
            tson::Layer *imageLayer = m->getLayer("Da Image Layer");
            tson::Layer *mainLayer = m->getLayer("Main Layer");
            tson::WangSet *wangset = m->getTileset("demo-tileset")->getWangset("wang-1");
            tson::WangColor *wangcolor = wangset->getColor("Britt");
            tson::TiledClass *c1 = m->getTileset("demo-tileset")->getTile(1)->getClass();
            tson::TiledClass *c2 = m->getClass();
            tson::TiledClass *c3 = m->getTileset("demo-tileset")->getClass();
            tson::TiledClass *c4 = objectLayer->getClass();
            tson::TiledClass *c5 = imageLayer->getClass();
            tson::TiledClass *c6 = mainLayer->getClass();
            tson::TiledClass *c7 = wangset->getClass();
            tson::TiledClass *c8 = wangcolor->getClass();

            REQUIRE(c1 != nullptr);
            REQUIRE(c1->getName() == "DummyClass");
            REQUIRE(c1->get<bool>("isDummy"));
            REQUIRE(c2 != nullptr);
            REQUIRE(c2->getName() == "DummyClass");
            REQUIRE(c2->get<bool>("isDummy"));
            REQUIRE(c3 != nullptr);
            REQUIRE(c3->getName() == "DummyClass");
            REQUIRE(c3->get<bool>("isDummy"));
            REQUIRE(c4 != nullptr);
            REQUIRE(c4->getName() == "DummyClass");
            REQUIRE(c4->get<bool>("isDummy"));
            REQUIRE(c5 != nullptr);
            REQUIRE(c5->getName() == "DummyClass");
            REQUIRE(c5->get<bool>("isDummy"));
            REQUIRE(c6 != nullptr);
            REQUIRE(c6->getName() == "DummyClass");
            REQUIRE(c6->get<bool>("isDummy"));
            REQUIRE(c7 != nullptr);
            REQUIRE(c7->getName() == "DummyClass");
            REQUIRE(c7->get<bool>("isDummy"));
            REQUIRE(c8 != nullptr);
            REQUIRE(c8->getName() == "DummyClass");
            REQUIRE(c8->get<bool>("isDummy"));

            tson::TiledClass *objectClass = objectLayer->firstObj("TestObject")->getClass(); //Object is changed from default values

            REQUIRE(objectClass != nullptr);
            REQUIRE(objectClass->getName() == "Enemy");
            REQUIRE(objectClass->get<int>("hp") == 10);
            REQUIRE(objectClass->get<std::string>("name") == "Galderino");

            tson::TiledClass *objectClass2 = objectLayer->firstObj("TestObject2")->getClass(); //Object is unchanged
            REQUIRE(objectClass2 != nullptr);
            REQUIRE(objectClass2->getName() == "Enemy");
            REQUIRE(objectClass2->get<int>("hp") == 1);
            REQUIRE(objectClass2->get<std::string>("name").empty());

        }
    }
}