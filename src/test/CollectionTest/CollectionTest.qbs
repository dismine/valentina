import qbs.FileInfo
import qbs.File

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "Qt"; submodules: ["testlib", "gui", "printsupport"] }
    Depends { name: "VTestLib" }

    name: "CollectionTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "CollectionTest" : "collectionTest"
    targetName: buildconfig.appTarget
    type: base.concat("tst_files")

    files: [
      "qttestmainlambda.cpp",
      "tst_tapecommandline.cpp",
      "tst_valentinacommandline.cpp",
      "tst_tapecommandline.h",
      "tst_valentinacommandline.h",
    ]

    Group {
        name: "Tape test files"
        prefix: "tst_tape" + FileInfo.pathSeparator()
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
        prefix: "tst_valentina" + FileInfo.pathSeparator()
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
        prefix: FileInfo.joinPaths(project.sourceDirectory, "src", "app", "share",
                FileInfo.pathSeparator())
        files: [
            FileInfo.joinPaths("collection", "bra.val"),
            FileInfo.joinPaths("collection", "bra.vit"),
            FileInfo.joinPaths("collection", "jacketM1_52-176.val"),
            FileInfo.joinPaths("collection", "jacketM2_40-146.val"),
            FileInfo.joinPaths("collection", "jacketM3_40-146.val"),
            FileInfo.joinPaths("collection", "jacketM4_40-146.val"),
            FileInfo.joinPaths("collection", "jacketM5_30-110.val"),
            FileInfo.joinPaths("collection", "jacketM6_30-110.val"),
            FileInfo.joinPaths("collection", "pantsM1_52-176.val"),
            FileInfo.joinPaths("collection", "pantsM2_40-146.val"),
            FileInfo.joinPaths("collection", "pantsM7.val"),
            FileInfo.joinPaths("collection", "TShirt_test.val"),
            FileInfo.joinPaths("collection", "TestDart.val"),
            FileInfo.joinPaths("collection", "pattern_blusa.val"),
            FileInfo.joinPaths("collection", "blusa.vit"),
            FileInfo.joinPaths("collection", "PajamaTopWrap2.val"),
            FileInfo.joinPaths("collection", "Susan.vit"),
            FileInfo.joinPaths("collection", "Moulage_0.5_armhole_neckline.val"),
            FileInfo.joinPaths("collection", "0.7_Armhole_adjustment_0.10.val"),
            FileInfo.joinPaths("collection", "my_calculated_measurements_for_val.vit"),
            FileInfo.joinPaths("collection", "Keiko_skirt.val"),
            FileInfo.joinPaths("collection", "keiko.vit"),
            FileInfo.joinPaths("collection", "medidas_eli2015.vit"),
            FileInfo.joinPaths("collection", "pantalon_base_Eli.val"),
            FileInfo.joinPaths("collection", "Razmernye_priznaki_dlya_zhenskogo_zhaketa.vit"),
            FileInfo.joinPaths("collection", "IMK_Zhaketa_poluprilegayuschego_silueta.val"),
            FileInfo.joinPaths("collection", "Lara_Jil.vit"),
            FileInfo.joinPaths("collection", "modell_2.val"),
            FileInfo.joinPaths("collection", "MaleShirt", "MaleShirt.val"),
            FileInfo.joinPaths("collection", "MaleShirt", "MaleShirt.vit"),
            FileInfo.joinPaths("collection", "Trousers", "Trousers.val"),
            FileInfo.joinPaths("collection", "Trousers", "trousers.vit"),
            FileInfo.joinPaths("collection", "Steampunk_trousers.val"),
            FileInfo.joinPaths("collection", "Steampunk_trousers.vit"),
            FileInfo.joinPaths("collection", "Basic_block_women-2016.val"),
            FileInfo.joinPaths("collection", "Gent_Jacket_with_tummy.val"),
            FileInfo.joinPaths("tables", "multisize", "GOST_man_ru.vst")
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
