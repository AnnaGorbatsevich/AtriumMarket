#!/usr/bin/env python3

import sys
import xml.etree.ElementTree as ET


def main() -> int:
    sys.stdout.reconfigure(encoding="utf-8")

    if len(sys.argv) != 3:
        print("usage: junit_to_summary.py <junit.xml> <heading>", file=sys.stderr)
        return 2

    xml_path, heading = sys.argv[1], sys.argv[2]

    try:
        root = ET.parse(xml_path).getroot()
    except (ET.ParseError, OSError) as exc:
        print(f"### {heading}\n\n⚠️ Could not read `{xml_path}`: {exc}")
        return 0

    suites = root.findall("testsuite") if root.tag == "testsuites" else [root]

    rows = []
    passed = failed = skipped = 0
    for suite in suites:
        for case in suite.findall("testcase"):
            name = case.get("name", "?")
            classname = case.get("classname", "")
            full_name = f"{classname}::{name}" if classname else name
            time = case.get("time", "0")

            if case.find("failure") is not None or case.find("error") is not None:
                failed += 1
                rows.append(f"| ❌ | `{full_name}` | {time}s | FAILED |")
            elif case.find("skipped") is not None:
                skipped += 1
                rows.append(f"| ⏭️ | `{full_name}` | {time}s | skipped |")
            else:
                passed += 1
                rows.append(f"| ✅ | `{full_name}` | {time}s | passed |")

    total = passed + failed + skipped
    status = "✅" if failed == 0 and total > 0 else ("⚠️" if total == 0 else "❌")

    print(f"### {status} {heading} — {passed}/{total} passed"
          + (f", {failed} failed" if failed else "")
          + (f", {skipped} skipped" if skipped else ""))
    print()
    if rows:
        print("| | Test | Time | Result |")
        print("|---|---|---|---|")
        print("\n".join(rows))
    else:
        print("_No test cases found in the report._")
    print()
    return 0


if __name__ == "__main__":
    sys.exit(main())
