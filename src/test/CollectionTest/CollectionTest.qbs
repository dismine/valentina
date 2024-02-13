import qbs.FileInfo
import qbs.File
import qbs.Utilities

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "Qt"; submodules: ["core", "testlib", "gui", "printsupport"] }
    Depends { name: "VTestLib" }
    Depends { name: "autotest" }

    Depends {
        name: "xerces-c"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && !buildconfig.useConanPackages
    }

    Depends {
        name: "conan.XercesC"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && buildconfig.useConanPackages
    }


    name: "CollectionTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "CollectionTest" : "collectionTest"
    targetName: buildconfig.appTarget
    type: base.concat("tst_files")
    autotest.workingDir: product.buildDirectory

    files: [
      "qttestmainlambda.cpp",
      "tst_tapecommandline.cpp",
      "tst_valentinacommandline.cpp",
      "tst_tapecommandline.h",
      "tst_valentinacommandline.h",
    ]

    Group {
        name: "Tape test files"
        prefix: "tst_tape/"
        files: [
            "keiko.vit",
            "empty.vit",
            "all_measurements_v0.3.0.vit",
            "all_measurements_v0.4.0.vst",
            "GOST_man_ru_v0.3.0.vst",
            "all_measurements_v0.3.3.vit",
            "all_measurements_v0.4.2.vst",
            "GOST_man_ru_v0.4.2.vst",
            "broken1.vit",
            "broken2.vit",
            "broken3.vit",
            "broken4.vit",
            "text.vit",
            "text.vst"
        ]
        fileTags: ["tst_tape_files"]
    }

    Group {
        name: "Valentina test files"
        prefix: "tst_valentina/"
        files: [
            "empty.val",
            "issue_372.val",
            "wrong_obj_type.val",
            "text.val",
            "issue_256.val",
            "issue_256_wrong_path.val",
            "issue_256_correct.vit",
            "issue_256_wrong.vit",
            "issue_256_correct.vst",
            "wrong_formula.val",
            "test_pedantic.val"
        ]
        fileTags: ["tst_valentina_files"]
    }

    Group {
        name: "Collection files"
        prefix: project.sourceDirectory + "/src/app/share/"
        files: [
            "collection/bra.val",
            "collection/bra.vit",
            "collection/jacketM1_52-176.val",
            "collection/jacketM2_40-146.val",
            "collection/jacketM3_40-146.val",
            "collection/jacketM4_40-146.val",
            "collection/jacketM5_30-110.val",
            "collection/jacketM6_30-110.val",
            "collection/pantsM1_52-176.val",
            "collection/pantsM2_40-146.val",
            "collection/pantsM7.val",
            "collection/TShirt_test.val",
            "collection/TestDart.val",
            "collection/pattern_blusa.val",
            "collection/blusa.vit",
            "collection/PajamaTopWrap2.val",
            "collection/Susan.vit",
            "collection/Moulage_0.5_armhole_neckline.val",
            "collection/0.7_Armhole_adjustment_0.10.val",
            "collection/my_calculated_measurements_for_val.vit",
            "collection/Keiko_skirt.val",
            "collection/keiko.vit",
            "collection/medidas_eli2015.vit",
            "collection/pantalon_base_Eli.val",
            "collection/Razmernye_priznaki_dlya_zhenskogo_zhaketa.vit",
            "collection/IMK_Zhaketa_poluprilegayuschego_silueta.val",
            "collection/Lara_Jil.vit",
            "collection/modell_2.val",
            "collection/MaleShirt/MaleShirt.val",
            "collection/MaleShirt/MaleShirt.vit",
            "collection/Trousers/Trousers.val",
            "collection/Trousers/trousers.vit",
            "collection/Steampunk_trousers.val",
            "collection/Steampunk_trousers.vit",
            "collection/Basic_block_women-2016.val",
            "collection/Gent_Jacket_with_tummy.val",
            "tables/multisize/GOST_man_ru.vst"
        ]
        fileTags: ["tst_collection_files"]
    }

    Rule {
        multiplex: true
//        alwaysRun: true
        inputs: ["tst_tape_files", "tst_valentina_files", "tst_collection_files"]
        outputFileTags: ["tst_files"]
        outputArtifacts: {
            var artifactNames = inputs["tst_valentina_files"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "tst_valentina", file.fileName);
            });

            artifactNames = artifactNames.concat(inputs["tst_collection_files"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "tst_valentina_collection", file.fileName);
            }));

            artifactNames = artifactNames.concat(inputs["tst_tape_files"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, "tst_tape", file.fileName);
            }));

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["tst_files"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing Valentina test file";
            cmd.highlight = "filegen";

            var sources = inputs["tst_valentina_files"].map(function(file) {
                return file.filePath;
            });

            sources = sources.concat(inputs["tst_collection_files"].map(function(file) {
                return file.filePath;
            }));

            sources = sources.concat(inputs["tst_tape_files"].map(function(file) {
                return file.filePath;
            }));

            cmd.sources = sources;

            var destination = inputs["tst_valentina_files"].map(function(file) {
                return FileInfo.joinPaths(product.buildDirectory, "tst_valentina", file.fileName);
            });

            destination = destination.concat(inputs["tst_collection_files"].map(function(file) {
                return FileInfo.joinPaths(product.buildDirectory, "tst_valentina_collection", file.fileName);
            }));

            destination = destination.concat(inputs["tst_tape_files"].map(function(file) {
                return FileInfo.joinPaths(product.buildDirectory, "tst_tape", file.fileName);
            }));

            cmd.destination = destination;

            cmd.sourceCode = function() {
                for (var i in sources) {
                    File.copy(sources[i], destination[i]);
                }
            };
            return [cmd];
        }
    }
}
