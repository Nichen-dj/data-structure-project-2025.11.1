document.addEventListener('DOMContentLoaded', function() {
    // DOM元素获取（保持不变）
    const searchInput = document.getElementById('search-input');
    const searchButton = document.getElementById('search-button');
    const suggestionsContainer = document.getElementById('suggestions');
    const resultsContainer = document.getElementById('results');
    const resultsStats = document.getElementById('results-stats');
    const loadingIndicator = document.getElementById('loading');
    const noResultsIndicator = document.getElementById('no-results');

    // API配置（保持不变）
    const API_BASE_URL = 'http://localhost:8080';
    const SEARCH_API = `${API_BASE_URL}/search`;
    const SUGGEST_API = `${API_BASE_URL}/suggest`;

    // 【新增】搜索历史相关方法
    const loadSearchHistory = () => {
        const history = localStorage.getItem('searchHistory');
        return history ? JSON.parse(history) : [];
    };

    const saveSearchHistory = (query) => {
        if (!query.trim()) return;
        const history = loadSearchHistory();
        const newHistory = [query, ...history.filter(item => item !== query)].slice(0, 10);
        localStorage.setItem('searchHistory', JSON.stringify(newHistory));
    };

    // 【修改】仅在无推荐时显示历史记录
    const showHistoryIfNoSuggestions = (hasSuggestions) => {
        if (!hasSuggestions) {
            const history = loadSearchHistory();
            if (history.length > 0) {
                history.forEach(query => {
                    const item = document.createElement('div');
                    item.className = 'suggestion-item';
                    item.textContent = query;
                    item.addEventListener('click', () => {
                        searchInput.value = query;
                        suggestionsContainer.innerHTML = '';
                        performSearch(query);
                    });
                    suggestionsContainer.appendChild(item);
                });
            } else {
                suggestionsContainer.innerHTML = '<div class="suggestion-item">无匹配建议</div>';
            }
        }
    };

    // 防抖函数（保持不变）
    function debounce(func, wait = 300) {
        let timeoutId;
        return function(...args) {
            clearTimeout(timeoutId);
            timeoutId = setTimeout(() => func.apply(this, args), wait);
        };
    }

    // 获取搜索建议（恢复多字母推荐逻辑）
    const fetchSuggestions = debounce(async function(query) {
        suggestionsContainer.innerHTML = '';
        const trimmedQuery = query.trim();
        
        // 【修改】输入为空时显示历史记录，有输入时正常加载推荐
        if (trimmedQuery.length === 0) {
            showHistoryIfNoSuggestions(false); // 强制显示历史
            return;
        }
        
        // 【恢复】保留原有多字母推荐触发逻辑（输入长度≥2时请求推荐）
        if (trimmedQuery.length < 2) return;

        try {
            const response = await fetch(`${SUGGEST_API}?q=${encodeURIComponent(trimmedQuery)}`);
            if (!response.ok) throw new Error(`HTTP错误：${response.status}`);
            
            const suggestions = await response.json();
            
            // 【修改】有推荐显示推荐，无推荐显示历史
            if (suggestions.length > 0) {
                suggestions.forEach(suggestion => {
                    const item = document.createElement('div');
                    item.className = 'suggestion-item';
                    const highlighted = suggestion.replace(
                        new RegExp(`^${trimmedQuery}`, 'i'),
                        match => `<strong>${match}</strong>`
                    );
                    item.innerHTML = highlighted;
                    
                    item.addEventListener('click', () => {
                        searchInput.value = suggestion;
                        suggestionsContainer.innerHTML = '';
                        performSearch(suggestion);
                    });
                    
                    suggestionsContainer.appendChild(item);
                });
                showHistoryIfNoSuggestions(true); // 有推荐，不显示历史
            } else {
                showHistoryIfNoSuggestions(false); // 无推荐，显示历史
            }
        } catch (error) {
            console.error('获取建议失败：', error);
            showHistoryIfNoSuggestions(false); // 出错时显示历史
        }
    });

    // 执行搜索（仅添加保存历史记录）
    async function performSearch(query) {
        const trimmedQuery = query.trim();
        if (!trimmedQuery) {
            alert('请输入查询词！');
            return;
        }

        saveSearchHistory(trimmedQuery); // 新增：保存历史

        // 以下代码完全不变（保留原有时间逻辑和搜索功能）
        loadingIndicator.style.display = 'block';
        resultsContainer.innerHTML = '';
        resultsStats.textContent = '';
        noResultsIndicator.style.display = 'none';

        try {
            const response = await fetch(`${SEARCH_API}?q=${encodeURIComponent(trimmedQuery)}`);
            if (!response.ok) throw new Error(`搜索请求失败：${response.statusText}`);
            
            const results = await response.json();
            const searchTime = (Math.random() * 0.5).toFixed(2);

            loadingIndicator.style.display = 'none';
            resultsStats.textContent = `找到 ${results.length} 个结果（搜索时间：${searchTime} 秒）`;

            if (results.length === 0) {
                noResultsIndicator.style.display = 'block';
                return;
            }

            results.forEach((result, index) => {
                const resultItem = document.createElement('div');
                resultItem.className = 'result-item';
                
                const highlightedPreview = result.preview.replace(
                    new RegExp(trimmedQuery, 'gi'),
                    match => `<strong>${match}</strong>`
                );

                resultItem.innerHTML = `
                    <div class="result-rank">${index + 1}</div>
                    <div class="result-content">
                        <div class="result-path">${result.doc_path}</div>
                        <div class="result-score">相关性得分：${result.score.toFixed(4)}</div>
                        <div class="result-preview">${highlightedPreview}</div>
                    </div>
                `;
                resultsContainer.appendChild(resultItem);
            });
        } catch (error) {
            loadingIndicator.style.display = 'none';
            resultsStats.textContent = '搜索过程中出现错误，请重试！';
            console.error('搜索失败：', error);
        }
    }

    // 事件监听（保持不变）
    searchInput.addEventListener('input', (e) => fetchSuggestions(e.target.value));
    searchButton.addEventListener('click', () => {
        performSearch(searchInput.value);
        suggestionsContainer.innerHTML = '';
    });
    searchInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            performSearch(searchInput.value);
            suggestionsContainer.innerHTML = '';
        }
    });
    document.addEventListener('click', (e) => {
        if (!searchInput.contains(e.target) && !suggestionsContainer.contains(e.target)) {
            suggestionsContainer.innerHTML = '';
        }
    });

    searchInput.focus();
});