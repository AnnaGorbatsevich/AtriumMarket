# AtriumMarket

## Тесты

Зайти в контейнер:

```bash
bash shell.sh
```

Дальше внутри:

```bash
cmake --build build -j$(nproc)                         # собрать всё


ctest --test-dir build --output-on-failure              # прогнать тесты всех сервисов


ctest --test-dir build -R gateway --output-on-failure    # только тесты gateway


build/gateway/runtests-gateway -k test_ping -v --no-header -p no:cacheprovider --color=yes                        # один конкретный тест, читаемый вывод
```

